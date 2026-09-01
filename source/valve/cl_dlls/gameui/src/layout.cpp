#include "layout.h"
#include "log.h"
#include "bgswitch.h"
#include <math.h>
#include <string.h>

/* All addresses below are RVAs into GameUI.dll (ImageBase 0x10000000),
 * recovered via Ghidra static analysis of the Nov-2020 build shipped with
 * this specific game install. They WILL be wrong for a different build --
 * this is a hand-tuned hack, not a portable technique. */

typedef void(__thiscall *SetPosFn)(void *item, int x, int y);
typedef void(__thiscall *SetSizeFn)(void *self, int wide, int tall);
typedef void(__thiscall *GetSizeFn)(void *self, int *outWide, int *outTall);
typedef char(__thiscall *IsVisibleFn)(void *item);
typedef void(__thiscall *SetColorFn)(void *self, unsigned int packedRgba);
typedef void(__thiscall *SetIntFn)(void *self, int value);
typedef void(__thiscall *SetBoolFn)(void *self, unsigned char value);
typedef void *(*GetSchemeFn)(void); /* FUN_1003f030 -- plain function, not virtual */
typedef void *(__thiscall *SchemeGetImageFn)(void *scheme, const char *path, int hardwareFiltered);
typedef void(__thiscall *SetImageAtIndexFn)(void *item, int index, void *image, int preOffset);
typedef void(__thiscall *SetTextInsetFn)(void *item, int xInset, int yInset);
typedef void(__thiscall *SetTextImageIndexFn)(void *item, int newIndex);
typedef void(__thiscall *GetContentSizeFn)(void *item, int *outWide, int *outTall);

static SetPosFn g_SetPos = NULL;
static SetSizeFn g_SetSize = NULL;
static GetSizeFn g_GetSize = NULL;
static SetColorFn g_SetBgColor = NULL;
static SetIntFn g_SetBackgroundTypeCandidate = NULL;
static SetBoolFn g_SetFlag40 = NULL;
static SetBoolFn g_SetFlag41 = NULL;
static SetBoolFn g_SetFlag42 = NULL;
static GetSchemeFn g_GetScheme = NULL;

#define RVA_SETPOS     0x000436f0u
#define RVA_SETSIZE    0x00043750u
#define RVA_GETSIZE    0x00043780u /* Panel::GetSize(int&,int&); sits immediately after SetSize in Panel.cpp's own method order, confirmed by matching thiscall(this,int*,int*) shape */
#define RVA_SETBGCOLOR 0x0006bcc0u /* vgui2::Menu::SetBgColor, confirmed via "Menu/BgColor" xref in ApplySchemeSettings */
#define RVA_SETBGTYPE_CANDIDATE 0x00046130u /* unconfirmed guess: setter/getter pair at offset 0x24, testing as PaintBackgroundType */
#define RVA_SETFLAG40  0x000467f0u /* vtable idx 62, stores 1 byte at this+0x40 -- unconfirmed */
#define RVA_SETFLAG41  0x00046800u /* vtable idx 63, stores 1 byte at this+0x41 -- unconfirmed */
#define RVA_SETFLAG42  0x00046810u /* vtable idx 64, stores 1 byte at this+0x42 -- unconfirmed */
#define RVA_GETSCHEME  0x0003f030u /* returns IScheme*-like singleton; confirmed via icon-loading pattern in a Career-mode button ctor */

/* vgui2::Label virtuals on CGameMenuItem's own vtable (base 0x1009681c),
 * confirmed by decompiling the item's ApplySchemeSettings (references
 * "Marlett") which calls SetImageAtIndex(0, m_pCheck/m_pBlankCheck, 6)
 * at exactly this offset. */
#define ITEM_VTABLE_SCHEME_GETIMAGE_OFFSET   0x14  /* on the scheme object, not the item */
#define ITEM_VTABLE_SETIMAGEATINDEX_OFFSET   0x25c
#define ITEM_VTABLE_SETTEXTINSET_OFFSET      0x230 /* same ApplySchemeSettings, reads "Menu/TextInset" */
#define ITEM_VTABLE_SETTEXTIMAGEINDEX_OFFSET 0x278 /* confirmed against real vgui2::Label::SetTextImageIndex source: moves
                                                     * the text image pointer (this+0x78) into the images-at-index array
                                                     * at the given slot, freeing index 0 for our icon */
#define ITEM_VTABLE_GETCONTENTSIZE_OFFSET    0x228 /* vgui2::Label::GetContentSize(int&,int&) -- confirmed by decompiling
                                                     * the ORIGINAL, unpatched Menu::PerformLayout at RVA 0x6afb0: its own
                                                     * auto-width pass (helper FUN_1006b1b0) calls exactly this vtable
                                                     * slot on each item with two int* out-params and takes the max
                                                     * (+8 padding) -- the real engine's own localization-aware content
                                                     * measurement, not something we're inventing. */

/* field offsets in DWORDs from 'this', read off the decompiled
 * FUN_1006afb0 body */
#define OFF_ITEM_COUNT     0x2f /* param_1[0x2f]: number of menu items */
#define OFF_ITEM_HEIGHT    0x1e /* param_1[0x1e]: per-item row height   */
#define OFF_ITEM_PTR_BASE  0x23 /* param_1[0x23]: base of item slot table (12 bytes/slot) */
#define OFF_ITEM_INDEX_MAP 0x2c /* param_1[0x2c]: loop-index -> slot-index remap array */

#define ITEM_SLOT_STRIDE   0xc
#define ITEM_VTABLE_IS_VISIBLE_OFFSET 0x78

/* Panel::_panelName. Found by decompiling FUN_10043660 (RVA 0x43660):
 * it frees any existing string at this+0x44 then heap-copies the new one
 * there -- the classic free+strdup shape of a SetName(const char*) setter.
 * The main menu panel is always constructed as
 * new CGameMenu(parent, datafile->GetName()) where datafile is loaded from
 * GameMenu.res, whose root key is literally "GameMenu" -- so reading this
 * field directly lets us tell the real main menu apart from every other
 * vgui2::Menu instance (combo box dropdowns, right-click menus, etc.) that
 * also runs through this same hooked layout routine. */
#define OFF_PANEL_NAME 0x44
#define MAIN_MENU_PANEL_NAME "GameMenu"

void LayoutHook_Init(HMODULE hOriginalGameUI)
{
    BYTE *base = (BYTE *)hOriginalGameUI;
    g_SetPos = (SetPosFn)(base + RVA_SETPOS);
    g_SetSize = (SetSizeFn)(base + RVA_SETSIZE);
    g_GetSize = (GetSizeFn)(base + RVA_GETSIZE);
    g_SetBgColor = (SetColorFn)(base + RVA_SETBGCOLOR);
    g_SetBackgroundTypeCandidate = (SetIntFn)(base + RVA_SETBGTYPE_CANDIDATE);
    g_SetFlag40 = (SetBoolFn)(base + RVA_SETFLAG40);
    g_SetFlag41 = (SetBoolFn)(base + RVA_SETFLAG41);
    g_SetFlag42 = (SetBoolFn)(base + RVA_SETFLAG42);
    g_GetScheme = (GetSchemeFn)(base + RVA_GETSCHEME);
    HookLog("LayoutHook_Init: base=%p g_SetPos=%p g_SetSize=%p g_GetSize=%p g_SetBgColor=%p g_SetBackgroundTypeCandidate=%p flags=%p/%p/%p g_GetScheme=%p",
            (void *)base, (void *)g_SetPos, (void *)g_SetSize, (void *)g_GetSize, (void *)g_SetBgColor, (void *)g_SetBackgroundTypeCandidate,
            (void *)g_SetFlag40, (void *)g_SetFlag41, (void *)g_SetFlag42, (void *)g_GetScheme);
}

static int ItemIsVisible(void *item)
{
    HookLog("  ItemIsVisible: item=%p", item);
    void **vtable = *(void ***)item;
    HookLog("  ItemIsVisible: vtable=%p", (void *)vtable);
    IsVisibleFn fn = (IsVisibleFn)vtable[ITEM_VTABLE_IS_VISIBLE_OFFSET / sizeof(void *)];
    HookLog("  ItemIsVisible: fn=%p, calling it", (void *)fn);
    int result = fn(item) != 0;
    HookLog("  ItemIsVisible: result=%d", result);
    return result;
}

static void GetItemContentSize(void *item, int *outWide, int *outTall)
{
    *outWide = 0;
    *outTall = 0;
    if (item == NULL) {
        return;
    }
    void **vtable = *(void ***)item;
    GetContentSizeFn fn = (GetContentSizeFn)vtable[ITEM_VTABLE_GETCONTENTSIZE_OFFSET / sizeof(void *)];
    fn(item, outWide, outTall);
    HookLog("  GetItemContentSize: item=%p wide=%d tall=%d", item, *outWide, *outTall);
}

static void AttachIcon(void *item, const char *iconPath)
{
    if (g_GetScheme == NULL) {
        return;
    }

    void *scheme = g_GetScheme();
    HookLog("  AttachIcon: scheme=%p path=%s", scheme, iconPath);
    if (scheme == NULL) {
        return;
    }

    void **schemeVtable = *(void ***)scheme;
    SchemeGetImageFn getImage = (SchemeGetImageFn)schemeVtable[ITEM_VTABLE_SCHEME_GETIMAGE_OFFSET / sizeof(void *)];
    void *image = getImage(scheme, iconPath, 1);
    HookLog("  AttachIcon: image=%p", image);
    if (image == NULL) {
        return;
    }

    void **itemVtable = *(void ***)item;

    /* Confirmed against the real vgui2::Label/MenuItem source: a plain
     * (non-checkable) MenuItem never moves its own text off image index 0,
     * so index 0 is "occupied" by the text by default. MenuItem itself
     * only calls SetTextImageIndex(1) for checkable items to make room for
     * the check glyph -- we do the same thing here to make room for our
     * icon instead. */
    SetTextImageIndexFn setTextImageIndex =
        (SetTextImageIndexFn)itemVtable[ITEM_VTABLE_SETTEXTIMAGEINDEX_OFFSET / sizeof(void *)];
    setTextImageIndex(item, 1);
    HookLog("  AttachIcon: SetTextImageIndex(1) done");

    /* Confirmed via disassembly: RET 0xc, i.e. genuinely 3 explicit stack
     * args -- our (index, image, preOffset) call was correct all along. */
    SetImageAtIndexFn setImg = (SetImageAtIndexFn)itemVtable[ITEM_VTABLE_SETIMAGEATINDEX_OFFSET / sizeof(void *)];
    setImg(item, 0, image, 4);
    HookLog("  AttachIcon: SetImageAtIndex done");
}

static void LayoutHook_Inner(void *thisPtr)
{
    HookLog("LayoutHook_ReplacementEntry: ENTER thisPtr=%p", thisPtr);

    int *self = (int *)thisPtr;
    int itemCount = self[OFF_ITEM_COUNT];
    int itemHeight = self[OFF_ITEM_HEIGHT];
    BYTE *itemSlotBase = (BYTE *)self[OFF_ITEM_PTR_BASE];
    int *indexMap = (int *)self[OFF_ITEM_INDEX_MAP];

    HookLog("LayoutHook_ReplacementEntry: itemCount=%d itemHeight=%d itemSlotBase=%p indexMap=%p",
            itemCount, itemHeight, (void *)itemSlotBase, (void *)indexMap);

    if (itemCount <= 0 || itemCount > 64 || g_SetPos == NULL) {
        HookLog("LayoutHook_ReplacementEntry: bailing out (itemCount out of sane range or g_SetPos unset)");
        return;
    }

    /* First pass: resolve slots and keep only the items that are actually
     * visible (GameMenu.res hides e.g. ResumeGame/Disconnect outside a
     * game) -- the circle must be divided among THESE, not the raw total,
     * or a menu with lots of hidden entries only fills a slice of it. */
    void *visibleItems[64];
    int visibleCount = 0;
    int i;
    for (i = 0; i < itemCount; i++) {
        int slotIndex = indexMap[i];
        HookLog("LayoutHook_ReplacementEntry: i=%d slotIndex=%d", i, slotIndex);

        if (slotIndex < 0 || slotIndex >= itemCount) {
            HookLog("LayoutHook_ReplacementEntry: i=%d slotIndex out of [0,%d) range, skipping", i, itemCount);
            continue;
        }

        void *item = *(void **)(itemSlotBase + slotIndex * ITEM_SLOT_STRIDE);
        HookLog("LayoutHook_ReplacementEntry: i=%d item=%p", i, item);

        if (item == NULL || !ItemIsVisible(item)) {
            continue;
        }

        visibleItems[visibleCount++] = item;
    }

    if (visibleCount <= 0) {
        HookLog("LayoutHook_ReplacementEntry: no visible items, nothing to lay out");
        return;
    }

    /* This same layout routine is the ONLY implementation left in the
     * process for vgui2::Menu::PerformLayout -- it runs for every Menu
     * instance, not just the main menu (combo box dropdowns, right-click
     * context menus, etc). Only the real main menu panel should get the
     * custom sidebar/icon treatment; everything else falls through to a
     * plain generic vertical stack that leaves size/position/icons alone
     * as much as possible. */
    const char *panelName = *(const char **)((char *)thisPtr + OFF_PANEL_NAME);
    int isMainMenu = (panelName != NULL && strcmp(panelName, MAIN_MENU_PANEL_NAME) == 0);
    HookLog("LayoutHook_ReplacementEntry: panelName=%s isMainMenu=%d", panelName != NULL ? panelName : "(null)", isMainMenu);

    if (isMainMenu) {
        /* By now the engine has fully applied whatever video mode the
         * player picked (unlike DLL-load time, when the desktop hadn't
         * necessarily switched into it yet), so this is the right moment
         * to pick the matching background tile set. No-ops after the
         * first call this process. */
        BgSwitch_RunOnceIfNeeded();
    }

    if (!isMainMenu) {
        /* Measure each item's own natural (localization-aware) content
         * size -- same real vgui2::Label::GetContentSize the stock engine
         * itself uses -- instead of trusting whatever width the panel
         * happened to have. (Briefly removed while chasing an unrelated
         * "closes right after reopening with a new resolution" report;
         * turned out not to be the cause, so it's back.) */
        int itemHeight = self[OFF_ITEM_HEIGHT];
        if (itemHeight <= 0) {
            itemHeight = 20;
        }

        int maxContentWide = 0;
        int maxContentTall = 0;
        int gv;
        for (gv = 0; gv < visibleCount; gv++) {
            int cw = 0, ct = 0;
            GetItemContentSize(visibleItems[gv], &cw, &ct);
            if (cw > maxContentWide) maxContentWide = cw;
            if (ct > maxContentTall) maxContentTall = ct;
        }
        if (maxContentTall + 4 > itemHeight) {
            itemHeight = maxContentTall + 4;
        }

        const int genericMarginX = 2;
        int genericItemWide = (maxContentWide > 0) ? (maxContentWide + 8) : 0;

        if (genericItemWide <= 0 && g_GetSize != NULL) {
            /* Measurement failed for some reason -- fall back to whatever
             * width the panel already has rather than collapsing it to 0. */
            int curWide = 0, curTall = 0;
            g_GetSize(thisPtr, &curWide, &curTall);
            genericItemWide = (curWide > genericMarginX * 2) ? (curWide - genericMarginX * 2) : 0;
        }

        HookLog("LayoutHook_ReplacementEntry: generic menu, itemHeight=%d maxContentWide=%d genericItemWide=%d",
                itemHeight, maxContentWide, genericItemWide);

        int gy = 0;
        for (gv = 0; gv < visibleCount; gv++) {
            g_SetPos(visibleItems[gv], genericMarginX, gy);
            if (g_SetSize != NULL && genericItemWide > 0) {
                g_SetSize(visibleItems[gv], genericItemWide, itemHeight);
            }
            gy += itemHeight;
        }

        if (g_SetSize != NULL && genericItemWide > 0) {
            g_SetSize(thisPtr, genericItemWide + genericMarginX * 2, gy);
        }

        HookLog("LayoutHook_ReplacementEntry: EXIT (generic menu path)");
        return;
    }

    /* Plain vertical list, sidebar-style: icon+label pairs stacked top to
     * bottom, not a circle -- anchored to the LEFT side of the screen.
     * The panel's own origin is fixed relative to the screen's top-left
     * corner regardless of resolution; a fixed offset from the right edge
     * would drift as the window is resized/the resolution changes, since
     * only the right/bottom edges move. Left-anchoring is resolution-safe. */
    const int marginX = 8;
    const int startY = 20; /* local coords are clipped below 0 by the parent (confirmed: negative startY just cut the first rows off) -- move the PANEL itself instead, see below */
    const int bottomPadding = 20;
    const int rowSpacing = 10;
    const int minRowHeight = 36; /* icon art is 32x32 -- never go below that plus a little headroom */
    int v;

    /* Only the main menu (4 visible items, in GameMenu.res order: New Game,
     * Find Servers, Options, Quit) gets real per-item icons -- other menu
     * variants (e.g. the 3-item in-game pause menu) would mismatch this
     * list, so they just don't get icons attached. */
    static const char *kMainMenuIcons[4] = {
        "gfx/vgui/icon_newgame",
        "gfx/vgui/icon_find",
        "gfx/vgui/icon_options",
        "gfx/vgui/icon_quit",
    };

    HookLog("LayoutHook_ReplacementEntry: visibleCount=%d (vertical list mode)", visibleCount);

    /* Attach icons FIRST -- they add to each item's own image list, which
     * its content-size measurement below has to see, or the measurement
     * would be taken before the icon exists. */
    if (visibleCount == 4) {
        for (v = 0; v < visibleCount; v++) {
            HookLog("LayoutHook_ReplacementEntry: v=%d attaching icon %s", v, kMainMenuIcons[v]);
            AttachIcon(visibleItems[v], kMainMenuIcons[v]);
        }
    }

    /* Measure the real, localization-aware content width/height of each
     * item (same GetContentSize the stock engine itself uses to auto-size
     * menus) instead of a hardcoded pixel width -- a longer translated
     * label just gets more room automatically. iconAllowance covers the
     * icon + gap that GetContentSize's own bookkeeping doesn't fully
     * attribute back to the label's reported width. */
    const int iconAllowance = 40;
    const int trailingPadding = 24;
    int maxContentWide = 0;
    int maxContentTall = 0;
    for (v = 0; v < visibleCount; v++) {
        int cw = 0, ct = 0;
        GetItemContentSize(visibleItems[v], &cw, &ct);
        HookLog("LayoutHook_ReplacementEntry: v=%d contentWide=%d contentTall=%d", v, cw, ct);
        if (cw > maxContentWide) maxContentWide = cw;
        if (ct > maxContentTall) maxContentTall = ct;
    }

    int itemWidth = (maxContentWide > 0) ? (maxContentWide + iconAllowance + trailingPadding) : 210;
    int rowHeight = (maxContentTall + 8 > minRowHeight) ? (maxContentTall + 8) : minRowHeight;
    HookLog("LayoutHook_ReplacementEntry: computed itemWidth=%d rowHeight=%d", itemWidth, rowHeight);

    /* The panel's own on-screen position (wherever the engine originally
     * put the stock vertical menu) is what actually needs to move to push
     * the list higher -- local child coordinates below 0 just get clipped
     * by this same panel, so that's a dead end. */
    HookLog("LayoutHook_ReplacementEntry: calling panel SetPos");
    g_SetPos(thisPtr, 0, 40);
    HookLog("LayoutHook_ReplacementEntry: panel SetPos returned");

    /* No backdrop this time -- plain list over the game background art,
     * matching the vanilla look. */

    /* Tried PaintBackgroundType=2 (no visible effect) and clearing flag
     * 0x41 (turned out to gate background painting entirely, not just the
     * border -- killed the backdrop). Neither got us rounded corners;
     * leaving both calls out until we're ready to do the real texture-
     * based approach. */

    int y = startY;
    for (v = 0; v < visibleCount; v++) {
        int x = marginX;

        HookLog("LayoutHook_ReplacementEntry: v=%d x=%d y=%d, calling SetPos", v, x, y);
        g_SetPos(visibleItems[v], x, y);
        HookLog("LayoutHook_ReplacementEntry: SetPos returned");

        if (g_SetSize != NULL) {
            HookLog("LayoutHook_ReplacementEntry: v=%d calling item SetSize %dx%d", v, itemWidth, rowHeight);
            g_SetSize(visibleItems[v], itemWidth, rowHeight);
            HookLog("LayoutHook_ReplacementEntry: item SetSize returned");
        }

        y += rowHeight + rowSpacing;
    }

    if (g_SetSize != NULL) {
        int totalHeight = y + bottomPadding;
        int totalWidth = marginX + itemWidth + bottomPadding;
        HookLog("LayoutHook_ReplacementEntry: calling SetSize %dx%d", totalWidth, totalHeight);
        g_SetSize(thisPtr, totalWidth, totalHeight);
        HookLog("LayoutHook_ReplacementEntry: SetSize returned");
    }

    HookLog("LayoutHook_ReplacementEntry: EXIT normally");
}

static volatile LONG g_disabledAfterCrash = 0;

static int CrashFilter(unsigned int code, EXCEPTION_POINTERS *ep)
{
    EXCEPTION_RECORD *rec = (ep != NULL) ? ep->ExceptionRecord : NULL;
    void *addr = (rec != NULL) ? rec->ExceptionAddress : NULL;

    if (rec != NULL && code == EXCEPTION_ACCESS_VIOLATION && rec->NumberParameters >= 2) {
        ULONG_PTR accessType = rec->ExceptionInformation[0]; /* 0=read, 1=write, 8=DEP */
        ULONG_PTR badVA = rec->ExceptionInformation[1];
        HookLog("LayoutHook_ReplacementEntry: *** SEH CAUGHT *** code=0x%08X instrAddr=%p accessType=%lu badAddr=0x%p",
                code, addr, (unsigned long)accessType, (void *)badVA);
    } else {
        HookLog("LayoutHook_ReplacementEntry: *** SEH CAUGHT *** code=0x%08X instrAddr=%p (no extra info)", code, addr);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

void __fastcall LayoutHook_ReplacementEntry(void *thisPtr)
{
    if (InterlockedCompareExchange(&g_disabledAfterCrash, 0, 0) != 0) {
        return; /* stay quiet after the first crash so the log doesn't explode from per-frame retries */
    }

    __try {
        LayoutHook_Inner(thisPtr);
    } __except (CrashFilter(GetExceptionCode(), GetExceptionInformation())) {
        HookLog("LayoutHook_ReplacementEntry: exception suppressed, disabling further attempts this run");
        InterlockedExchange(&g_disabledAfterCrash, 1);
    }
}
