#!/usr/bin/env python3
"""Add a static import to a 32-bit PE DLL so Windows loads our sidecar.

Always reads a pristine original (steam_api.dll) and writes a patched copy.
Does not modify the original. Strips Authenticode (it would be invalid
after any PE edit anyway).
"""
from __future__ import print_function

import argparse
import os
import struct
import sys


def align(value, alignment):
    return (value + alignment - 1) & ~(alignment - 1)


class Pe32(object):
    def __init__(self, data):
        self.data = bytearray(data)
        if self.data[:2] != b"MZ":
            raise ValueError("not an MZ file")
        self.e_lfanew = struct.unpack_from("<I", self.data, 0x3C)[0]
        if self.data[self.e_lfanew:self.e_lfanew + 4] != b"PE\0\0":
            raise ValueError("not a PE file")
        self.file_hdr = self.e_lfanew + 4
        self.opt = self.e_lfanew + 24
        magic = struct.unpack_from("<H", self.data, self.opt)[0]
        if magic != 0x10B:
            raise ValueError("only PE32 (i386) is supported, magic=0x%X" % magic)
        self.opthdr_size = struct.unpack_from("<H", self.data, self.file_hdr + 16)[0]
        self.section_table = self.opt + self.opthdr_size

    def u16(self, off):
        return struct.unpack_from("<H", self.data, off)[0]

    def u32(self, off):
        return struct.unpack_from("<I", self.data, off)[0]

    def set_u16(self, off, value):
        struct.pack_into("<H", self.data, off, value)

    def set_u32(self, off, value):
        struct.pack_into("<I", self.data, off, value)

    @property
    def number_of_sections(self):
        return self.u16(self.file_hdr + 2)

    @number_of_sections.setter
    def number_of_sections(self, value):
        self.set_u16(self.file_hdr + 2, value)

    @property
    def file_alignment(self):
        return self.u32(self.opt + 36)

    @property
    def section_alignment(self):
        return self.u32(self.opt + 32)

    def datadir(self, index):
        return self.u32(self.opt + 96 + index * 8), self.u32(self.opt + 96 + index * 8 + 4)

    def set_datadir(self, index, rva, size):
        self.set_u32(self.opt + 96 + index * 8, rva)
        self.set_u32(self.opt + 96 + index * 8 + 4, size)

    def section_header(self, i):
        return self.section_table + i * 40

    def section_info(self, i):
        off = self.section_header(i)
        name = bytes(self.data[off:off + 8]).split(b"\0")[0]
        vsz, va, rsz, rp = struct.unpack_from("<IIII", self.data, off + 8)
        ch = self.u32(off + 36)
        return name, vsz, va, rsz, rp, ch

    def rva_to_off(self, rva):
        for i in range(self.number_of_sections):
            _name, vsz, va, rsz, rp, _ch = self.section_info(i)
            span = max(vsz, rsz)
            if va <= rva < va + span:
                return rp + (rva - va)
        raise ValueError("RVA 0x%X not in any section" % rva)

    def existing_import_dlls(self):
        imp_rva, imp_sz = self.datadir(1)
        off = self.rva_to_off(imp_rva)
        names = []
        i = 0
        while True:
            _oft, _td, _fwd, name_rva, _ft = struct.unpack_from("<IIIII", self.data, off + i * 20)
            if name_rva == 0:
                break
            name_off = self.rva_to_off(name_rva)
            end = self.data.index(b"\0", name_off)
            names.append(bytes(self.data[name_off:end]).decode("ascii"))
            i += 1
        return names, i, off


def build_ep_stub(stub_rva, orig_ep_rva, iat_rva):
    """stdcall DllEntry wrapper: call original EP, then Vellum_Init.

    Must run AFTER GameUI's CRT/DllMain so PerformLayout is not hooked while
    GameUI's own constructors are still running.
    """
    code = bytearray()
    # Rebuild stdcall args and call original _DllMainCRTStartup.
    code += b"\xFF\x74\x24\x0C" * 3          # push [esp+12] x3
    call_at = len(code)
    code += b"\xE8\x00\x00\x00\x00"          # call orig_ep
    code += b"\x50"                          # push eax (keep BOOL result)
    code += b"\x83\x7C\x24\x0C\x01"          # cmp dword [esp+12], DLL_PROCESS_ATTACH
    jne_at = len(code)
    code += b"\x75\x00"                      # jne skip
    code += b"\x8B\x44\x24\x08"              # mov eax, [esp+8] ; hinst
    code += b"\xFF\x90" + struct.pack("<I", iat_rva)  # call [eax+IAT]
    skip_at = len(code)
    code += b"\x58"                          # pop eax
    code += b"\xC2\x0C\x00"                  # ret 12

    rel = orig_ep_rva - (stub_rva + call_at + 5)
    struct.pack_into("<i", code, call_at + 1, rel)
    disp = skip_at - (jne_at + 2)
    if not (-128 <= disp <= 127):
        raise SystemExit("EP stub jne displacement out of range")
    code[jne_at + 1] = disp & 0xFF
    return bytes(code)


def add_import(src_bytes, dll_name, func_name, wrap_ep=False):
    pe = Pe32(src_bytes)
    existing, n_desc, desc_off = pe.existing_import_dlls()
    if dll_name.lower() in [n.lower() for n in existing]:
        raise SystemExit("%s is already imported" % dll_name)

    # Strip Authenticode overlay; it cannot survive a PE edit.
    cert_rva, cert_sz = pe.datadir(4)
    if cert_sz:
        pe.set_datadir(4, 0, 0)
        if cert_rva > 0 and cert_rva < len(pe.data):
            pe.data = pe.data[:cert_rva]

    nsec = pe.number_of_sections
    last_va = 0
    last_va_end = 0
    last_raw_end = 0
    for i in range(nsec):
        _name, vsz, va, rsz, rp, _ch = pe.section_info(i)
        last_va = max(last_va, va)
        last_va_end = max(last_va_end, va + align(max(vsz, rsz), pe.section_alignment))
        last_raw_end = max(last_raw_end, rp + rsz)

    # Trim overlay/padding past the last raw section (leftover cert bytes).
    if last_raw_end < len(pe.data):
        pe.data = pe.data[:last_raw_end]
    pe.data.extend(b"\0" * (align(len(pe.data), pe.file_alignment) - len(pe.data)))
    last_raw_end = len(pe.data)

    new_va = last_va_end
    new_raw = last_raw_end

    # Layout inside the new section:
    #   import descriptors (copied original + new + null)
    #   ILT (func RVA, 0)
    #   IAT (func RVA, 0)
    #   IMAGE_IMPORT_BY_NAME
    #   dll name
    desc_bytes = n_desc * 20
    copied = bytes(pe.data[desc_off:desc_off + desc_bytes])

    hint_name = struct.pack("<H", 0) + func_name.encode("ascii") + b"\0"
    if len(hint_name) % 2:
        hint_name += b"\0"
    dll_bytes = dll_name.encode("ascii") + b"\0"

    # Place payload after the descriptor array (old + new + terminator).
    payload_off = (n_desc + 2) * 20
    ilt_off = payload_off
    iat_off = ilt_off + 8
    hint_off = iat_off + 8
    dll_off = hint_off + len(hint_name)

    orig_ep = pe.u32(pe.opt + 16)

    hint_rva = new_va + hint_off
    ilt = struct.pack("<II", hint_rva, 0)
    iat = struct.pack("<II", hint_rva, 0)
    iat_rva = new_va + iat_off

    new_desc = struct.pack(
        "<IIIII",
        new_va + ilt_off,  # OriginalFirstThunk
        0,                 # TimeDateStamp
        0,                 # ForwarderChain
        new_va + dll_off,  # Name
        iat_rva,           # FirstThunk
    )
    terminator = b"\0" * 20

    import_end = dll_off + len(dll_bytes)
    stub = b""
    stub_off = 0
    if wrap_ep:
        stub_off = align(import_end, 16)
        stub = build_ep_stub(new_va + stub_off, orig_ep, iat_rva)
        content_size = stub_off + len(stub)
        sect_chars = 0xE0000060  # CODE+IDATA | EXECUTE | READ | WRITE
    else:
        content_size = import_end
        sect_chars = 0xC0000040  # IDATA | READ | WRITE
    raw_size = align(content_size, pe.file_alignment)
    virt_size = align(content_size, pe.section_alignment)

    section = bytearray(raw_size)
    section[0:desc_bytes] = copied
    section[desc_bytes:desc_bytes + 20] = new_desc
    section[desc_bytes + 20:desc_bytes + 40] = terminator
    section[ilt_off:ilt_off + 8] = ilt
    section[iat_off:iat_off + 8] = iat
    section[hint_off:hint_off + len(hint_name)] = hint_name
    section[dll_off:dll_off + len(dll_bytes)] = dll_bytes
    if wrap_ep:
        section[stub_off:stub_off + len(stub)] = stub

    # Section header: 40 bytes, must still fit inside SizeOfHeaders.
    hdr_end = pe.section_table + (nsec + 1) * 40
    size_of_headers = pe.u32(pe.opt + 60)
    if hdr_end > size_of_headers:
        raise SystemExit("no room in PE headers for another section")

    sh = pe.section_header(nsec)
    name = b".himport"
    pe.data[sh:sh + 8] = name + b"\0" * (8 - len(name))
    struct.pack_into(
        "<IIIIIIHHI",
        pe.data,
        sh + 8,
        content_size,          # VirtualSize
        new_va,                # VirtualAddress
        raw_size,              # SizeOfRawData
        new_raw,               # PointerToRawData
        0, 0, 0, 0,
        sect_chars,
    )

    pe.number_of_sections = nsec + 1
    pe.set_u32(pe.opt + 56, new_va + virt_size)  # SizeOfImage
    pe.set_u32(pe.opt + 8, pe.u32(pe.opt + 8) + raw_size)  # SizeOfInitializedData
    if wrap_ep:
        pe.set_u32(pe.opt + 4, pe.u32(pe.opt + 4) + raw_size)  # SizeOfCode
        pe.set_u32(pe.opt + 16, new_va + stub_off)  # AddressOfEntryPoint
    pe.set_u32(pe.opt + 64, 0)  # CheckSum
    pe.set_datadir(1, new_va, (n_desc + 2) * 20)  # Import directory, includes terminator

    pe.data.extend(section)
    return bytes(pe.data)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("original")
    parser.add_argument("output")
    parser.add_argument("--dll", default="vellum.dll")
    parser.add_argument("--func", default="Vellum_Init")
    parser.add_argument("--wrap-ep", action="store_true",
                        help="Wrap the DLL entry point to call the imported func after DllMain")
    args = parser.parse_args()

    with open(args.original, "rb") as f:
        src = f.read()
    patched = add_import(src, args.dll, args.func, wrap_ep=args.wrap_ep)
    os.makedirs(os.path.dirname(os.path.abspath(args.output)) or ".", exist_ok=True)
    with open(args.output, "wb") as f:
        f.write(patched)
    print("patched %s -> %s (import %s!%s%s, %d -> %d bytes)" % (
        args.original, args.output, args.dll, args.func,
        ", EP wrapper" if args.wrap_ep else "", len(src), len(patched)))


if __name__ == "__main__":
    main()
