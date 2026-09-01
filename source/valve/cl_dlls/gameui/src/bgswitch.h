#ifndef BGSWITCH_H
#define BGSWITCH_H

/* Stashes the game install root (the folder containing cstrike/, valve/,
 * hw.dll, etc -- no trailing slash) for BgSwitch_RunOnceIfNeeded() to use
 * later. Call this early (right after resolving it in LoadOriginalAndHook);
 * it does no file I/O by itself. */
void BgSwitch_SetGameRoot(const char *gameRootDir);

/* Rewrites cstrike/resource/BackgroundLayout.txt and
 * BackgroundLoadingLayout.txt to point at the right pre-baked tile set
 * (widescreen vs 4:3), based on GetSystemMetrics at the time of the call --
 * meant to be called from inside the main menu's own layout pass (after
 * the engine has fully applied whatever video mode the player picked),
 * not at DLL-load time (too early: the desktop resolution hadn't been
 * switched into the target video mode yet, so every launch read back the
 * same stale value regardless of the resolution actually picked in
 * Options). No-ops after the first successful call this process. */
void BgSwitch_RunOnceIfNeeded(void);

#endif
