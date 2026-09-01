#ifndef LAYOUT_H
#define LAYOUT_H

#include <windows.h>

/* Resolves the real GameUI.dll's SetPos/SetSize helpers relative to its
 * loaded base address. Must be called once, right after LoadLibrary()'ing
 * the renamed original module and before installing the hook. */
void LayoutHook_Init(HMODULE hOriginalGameUI);

/* Replacement for CGameMenu's internal layout routine (was FUN_1006afb0 at
 * RVA 0x6afb0 in the Nov-2020 GameUI.dll build). Arranges menu items around
 * a circle instead of the stock vertical stack. Called via the same
 * register convention the original used (this in ECX, no other args). */
void __fastcall LayoutHook_ReplacementEntry(void *thisPtr);

#endif
