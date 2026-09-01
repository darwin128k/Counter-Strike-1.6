#include <windows.h>
#include <string.h>
#include "layout.h"
#include "log.h"
#include "bgswitch.h"

/* This DLL is meant to replace valve/cl_dlls/GameUI.dll after the real
 * file has been renamed to GameUI_orig.dll in the same folder. We load the
 * renamed original ourselves, patch one internal function in its memory
 * image (the menu layout routine), and transparently forward the single
 * real export (CreateInterface) so the engine notices nothing else. */

typedef void *(*CreateInterfaceFn)(const char *pName, int *pReturnCode);

static HMODULE g_hOriginal = NULL;
static CreateInterfaceFn g_pOriginalCreateInterface = NULL;

#define HOOK_TARGET_RVA 0x0006afb0u
#define HOOK_STUB_SIZE  5u /* E9 rel32 */

static void InstallLayoutHook(HMODULE hOriginal)
{
    BYTE *target = (BYTE *)hOriginal + HOOK_TARGET_RVA;
    DWORD oldProtect;

    HookLog("InstallLayoutHook: hOriginal=%p target=%p", (void *)hOriginal, (void *)target);

    if (!VirtualProtect(target, HOOK_STUB_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        HookLog("InstallLayoutHook: VirtualProtect FAILED, GetLastError=%lu", GetLastError());
        return;
    }

    LayoutHook_Init(hOriginal);
    HookLog("InstallLayoutHook: LayoutHook_Init done");

    INT_PTR rel = (INT_PTR)LayoutHook_ReplacementEntry - (INT_PTR)(target + HOOK_STUB_SIZE);
    HookLog("InstallLayoutHook: ReplacementEntry=%p rel=%ld", (void *)LayoutHook_ReplacementEntry, (long)rel);

    target[0] = 0xE9;
    *(INT_PTR *)(target + 1) = rel;

    VirtualProtect(target, HOOK_STUB_SIZE, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), target, HOOK_STUB_SIZE);
    HookLog("InstallLayoutHook: patch written and flushed");
}

static void LoadOriginalAndHook(void)
{
    char selfPath[MAX_PATH];
    char origPath[MAX_PATH];
    HMODULE hSelf = NULL;
    char *slash;

    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                        (LPCSTR)&LoadOriginalAndHook, &hSelf);
    GetModuleFileNameA(hSelf, selfPath, MAX_PATH);

    slash = strrchr(selfPath, '\\');
    if (slash != NULL) {
        *(slash + 1) = '\0';
    } else {
        selfPath[0] = '\0';
    }

    wsprintfA(origPath, "%sGameUI_orig.dll", selfPath);
    HookLog("LoadOriginalAndHook: loading '%s'", origPath);

    /* selfPath is ".../valve/cl_dlls/" -- strip those two path components
     * to get the game install root, which is what BgSwitch needs to find
     * cstrike/resource/BackgroundLayout.txt. */
    {
        char gameRoot[MAX_PATH];
        char *s;
        strcpy(gameRoot, selfPath);
        size_t len = strlen(gameRoot);
        if (len > 0 && gameRoot[len - 1] == '\\') {
            gameRoot[len - 1] = '\0';
        }
        s = strrchr(gameRoot, '\\');
        if (s != NULL) {
            *s = '\0';
        }
        s = strrchr(gameRoot, '\\');
        if (s != NULL) {
            *s = '\0';
        }
        HookLog("LoadOriginalAndHook: gameRoot='%s'", gameRoot);
        BgSwitch_SetGameRoot(gameRoot);

        /* Do this NOW, before LoadLibraryA below hands control back to the
         * engine to go on and load the background art -- registry values
         * (unlike GetSystemMetrics) are already correct this early, so
         * there's no reason to wait for the menu layout hook to fire.
         * Waiting caused a one-launch lag: the engine would load the
         * background from whatever the file said BEFORE our rewrite,
         * and only the NEXT restart would show the corrected pick. */
        BgSwitch_RunOnceIfNeeded();
    }

    g_hOriginal = LoadLibraryA(origPath);
    if (g_hOriginal == NULL) {
        HookLog("LoadOriginalAndHook: LoadLibraryA FAILED, GetLastError=%lu", GetLastError());
        return;
    }
    HookLog("LoadOriginalAndHook: loaded at base=%p", (void *)g_hOriginal);

    g_pOriginalCreateInterface = (CreateInterfaceFn)GetProcAddress(g_hOriginal, "CreateInterface");
    HookLog("LoadOriginalAndHook: original CreateInterface=%p", (void *)g_pOriginalCreateInterface);

    InstallLayoutHook(g_hOriginal);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD reason, LPVOID reserved)
{
    (void)reserved;
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        /* Deliberately do NOT LoadLibrary/hook here: DllMain for
         * DLL_PROCESS_ATTACH runs under the loader lock, and calling
         * LoadLibrary on a DLL with its own dependency chain from inside
         * that lock is a well-known way to deadlock or crash. Defer all
         * of that to the first real CreateInterface call instead, which
         * happens once the engine is already fully initialized. */
        DisableThreadLibraryCalls(hinstDLL);
        break;
    case DLL_PROCESS_DETACH:
        if (g_hOriginal != NULL) {
            FreeLibrary(g_hOriginal);
        }
        break;
    default:
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) void *CreateInterface(const char *pName, int *pReturnCode)
{
    HookLog("CreateInterface: called for '%s'", pName != NULL ? pName : "(null)");

    if (g_hOriginal == NULL) {
        LoadOriginalAndHook();
    }

    if (g_pOriginalCreateInterface == NULL) {
        HookLog("CreateInterface: no original CreateInterface available, returning failure");
        if (pReturnCode != NULL) {
            *pReturnCode = 1;
        }
        return NULL;
    }
    void *result = g_pOriginalCreateInterface(pName, pReturnCode);
    HookLog("CreateInterface: original returned %p", result);
    return result;
}
