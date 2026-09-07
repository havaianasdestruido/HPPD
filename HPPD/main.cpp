#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <cstdio>
#include <intrin.h>

#include "utils/utils.h"
#include "payloads.h"
#include "utils/seed.h"
#include "utils/vmdetect.h"
#include "utils/xmplayer.h"
#include "utils/bytebeats.h"
#include "easter/dates.h"
#include "easter/desk.h"
#include "easter/sys.h"

// ============================================================
// HPPD - effects demo
//
// No:
// - disk writes
// - MBR modification
// - privilege escalation
// - BSOD / NtRaiseHardError
// - audio playback
//
// Payloads 27, 28, 35, 36, 37, 39, 42 do manipulate windows
// the user owns (resize / move / disable Close / spawn a few
// harmless Windows utilities). They are still safe in the
// sense of "won't kill the machine", but they will fight the
// user while they run.
//
// ESC / STOP aborts Payloads 1-57.
// Payloads 1-12 retain their original finite durations
// unless aborted early.
//
// A global SEED is picked once at startup and printed to the
// console. Each payload derives its own variant from it, so
// every run of the same effects looks different.
//
// The start screen is a simple popup. Press Ctrl+F12 on it to
// open the payload selector: pick a payload in the drop-down,
// read its nickname, launch it with "Launch" (bottom-left) and
// stop it with "STOP" (bottom-right). "Continue" still runs
// the whole demo in order.
// ============================================================

// ============================================================
// PAYLOAD CATALOG (1-57)
// ============================================================

struct PayloadEntry
{
    const wchar_t* name;
    DWORD ms;
    LPTHREAD_START_ROUTINE fn;
    bool heavy; // full-screen per-pixel renderer: runs one at a time
};

static PayloadEntry gPayloads[] =
{
    { L"01 - Screen Feedback",          5000, GDIPayload1,  true  },
    { L"02 - Pixelation Zoom",          5000, GDIPayload2,  true  },
    { L"03 - Error Icons",              5000, GDIPayload3,  false },
    { L"04 - XOR Inversion",            5000, GDIPayload4,  false },
    { L"05 - Motion Blur",              5000, GDIPayload5,  true  },
    { L"06 - Hue Displacement",         5000, GDIPayload6,  true  },
    { L"07 - Plasma Hue",               5000, GDIPayload7,  true  },
    { L"08 - Fractal RGB",              5000, GDIPayload8,  true  },
    { L"09 - Zoom Hue Chaos",           5000, GDIPayload9,  true  },
    { L"10 - Color Pulse",              5000, GDIPayload10, true  },
    { L"11 - RGB Noise Zoom",           5000, GDIPayload11, true  },
    { L"12 - Horizontal Tear",          5000, GDIPayload12, false },
    { L"13 - Chromatic Plasma",         5000, GDIPayload13, true  },
    { L"14 - RGB Displacement",         5000, GDIPayload14, true  },
    { L"15 - Feedback Tunnel",          5000, GDIPayload15, true  },
    { L"16 - Fractured Screen",         5000, GDIPayload16, false },
    { L"17 - Neon Edge Inversion",      5000, GDIPayload17, true  },
    { L"18 - Liquid Ripple",            5000, GDIPayload18, false },
    { L"19 - Quantum Spiral",           5000, GDIPayload19, true  },
    { L"20 - Color Worm",               5000, GDIPayload20, true  },
    { L"21 - Radial Shockwave",         5000, GDIPayload21, true  },
    { L"22 - RGB Void",                 5000, GDIPayload22, true  },
    { L"23 - Retro CRT",                5000, GDIPayload23, true  },
    { L"24 - Absolute GDI Chaos",       5000, GDIPayload24, true  },
    { L"25 - Dark",                     5000, GDIPayload25, false },
    { L"26 - Cursors",                  5000, GDIPayload26, false },
    { L"27 - Programs",                 5000, GDIPayload27, false },
    { L"28 - Buttons",                  5000, GDIPayload28, false },
    { L"29 - RGBCircle",                5000, GDIPayload29, false },
    { L"30 - IconWave",                 5000, GDIPayload30, false },
    { L"31 - Sinewave",                 5000, GDIPayload31, false },
    { L"32 - Shake",                    5000, GDIPayload32, false },
    { L"33 - Dark2",                    5000, GDIPayload33, false },
    { L"34 - Melter",                   5000, GDIPayload34, false },
    { L"35 - Trails",                   5000, GDIPayload35, false },
    { L"36 - ResizeWindow",            5000, GDIPayload36, false },
    { L"37 - MoveWindow",              5000, GDIPayload37, false },
    { L"38 - Melter2",                 5000, GDIPayload38, false },
    { L"39 - WindowMax",               5000, GDIPayload39, false },
    { L"40 - Rotate",                  5000, GDIPayload40, true  },
    { L"41 - Rainbow",                 5000, GDIPayload41, false },
    { L"42 - RandomCur",               5000, GDIPayload42, false },
    { L"43 - Melter3",                 5000, GDIPayload43, false },
    { L"44 - Colors",                  5000, GDIPayload44, false },
    { L"45 - Flip",                    5000, GDIPayload45, false },
    { L"46 - CircleSquare",            5000, GDIPayload46, false },
    { L"47 - Bright",                  5000, GDIPayload47, false },
    { L"48 - ExtremeRotation",         5000, GDIPayload48, true  },
    { L"49 - Cubes",                   5000, GDIPayload49, false },
    { L"50 - Chromatic Vortex",        5000, GDIPayload50, true  },
    { L"51 - Polar Liquid",            5000, GDIPayload51, true  },
    { L"52 - Kaleidoscope",            5000, GDIPayload52, true  },
    { L"53 - Radial Tunnel",           5000, GDIPayload53, true  },
    { L"54 - Voronoi Cells",           5000, GDIPayload54, true  },
    { L"55 - Sine Fold",               5000, GDIPayload55, true  },
    { L"56 - Chromatic Aberration",    5000, GDIPayload56, true  },
    { L"57 - Mirror Feedback",         5000, GDIPayload57, true  },
    { L"58 - Echo Stack",              5000, GDIPayload58, true  },
    { L"59 - Interference",            5000, GDIPayload59, true  },
    { L"60 - Pixel Sort",              5000, GDIPayload60, true  },
    { L"61 - Julia Set",               5000, GDIPayload61, true  },
    { L"62 - Flow Field",              5000, GDIPayload62, true  },
    { L"63 - Reaction Diff",           5000, GDIPayload63, true  },
    { L"64 - 29 de Março",             5000, GDIPayload64, false },
};

static const int gPayloadCount =
    static_cast<int>(
        sizeof(gPayloads) /
        sizeof(gPayloads[0])
    );

// ============================================================
// MULTI-MODE FULL-SCREEN SLOT
//
// Full-screen per-pixel renderers (fractals, Julia, reaction
// diffusion, ...) share one slot so only a single one paints
// the whole screen at a time. Lightweight modifiers (shake,
// colors, windows, cursors) are NOT gated and overlap freely.
// ============================================================

namespace
{
    HANDLE gHeavySlot = nullptr;

    struct SlotArg
    {
        LPTHREAD_START_ROUTINE fn;
    };

    DWORD WINAPI SlotRunner(LPVOID param)
    {
        SlotArg* arg =
            static_cast<SlotArg*>(param);

        WaitForSingleObject(
            gHeavySlot,
            INFINITE
        );

        arg->fn(nullptr);

        ReleaseMutex(gHeavySlot);

        delete arg;

        return 0;
    }
}

// ============================================================
// WARNING POPUP + PAYLOAD SELECTOR
//
// Ctrl+F12 toggles the payload selector: a drop-down of every
// payload, a nickname label, "Launch" (bottom-left) and
// "STOP" (bottom-right). "Continue" runs the full demo.
// ============================================================

namespace
{
    const int IDC_COMBO    = 1001;
    const int IDC_NICK     = 1002;
    const int IDC_LAUNCH   = 1003;
    const int IDC_STOP     = 1004;
    const int IDC_CONTINUE = 1005;

    struct PopupState
    {
        HWND hwnd    = nullptr;
        HWND combo   = nullptr;
        HWND nick    = nullptr;
        HWND launch  = nullptr;
        HWND stop    = nullptr;
        HANDLE thread = nullptr;
        bool selector   = false;
        bool f12WasDown = false;
        int result      = 0; // 1 = continue demo, 0 = exit
    };

    PopupState gPop;
}

static void UpdateNick()
{
    const int sel =
        static_cast<int>(
            SendMessageW(
                gPop.combo,
                CB_GETCURSEL,
                0,
                0
                )
            );

    if (sel < 0 || sel >= gPayloadCount)
        return;

    wchar_t buf[256];

    wsprintfW(
        buf,
        L"Payload: %ls",
        gPayloads[sel].name
    );

    SetWindowTextW(gPop.nick, buf);
}

static void ToggleSelector()
{
    gPop.selector = !gPop.selector;

    ShowWindow(gPop.combo,   gPop.selector ? SW_SHOW : SW_HIDE);
    ShowWindow(gPop.nick,    gPop.selector ? SW_SHOW : SW_HIDE);
    ShowWindow(gPop.launch,  gPop.selector ? SW_SHOW : SW_HIDE);
    ShowWindow(gPop.stop,    gPop.selector ? SW_SHOW : SW_HIDE);

    if (gPop.selector)
    {
        if (SendMessageW(gPop.combo, CB_GETCURSEL, 0, 0) == CB_ERR)
        {
            SendMessageW(gPop.combo, CB_SETCURSEL, 0, 0);
            UpdateNick();
        }
    }
}

static void StopRunning()
{
    if (!gPop.thread)
        return;

    gFxStop = true;
    WaitEffect(gPop.thread);
    gPop.thread = nullptr;

    EnableWindow(gPop.combo,   TRUE);
    EnableWindow(gPop.launch,  TRUE);
    EnableWindow(gPop.stop,    FALSE);
}

static LRESULT CALLBACK PopupProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_COMBO:
            if (HIWORD(wParam) == CBN_SELCHANGE)
                UpdateNick();
            return 0;

        case IDC_LAUNCH:
        {
            const int sel =
                static_cast<int>(
                    SendMessageW(
                        gPop.combo,
                        CB_GETCURSEL,
                        0,
                        0
                        )
                    );

            if (sel < 0 || gPop.thread)
                return 0;

            gFxStop = false;

            gPop.thread =
                StartEffect(
                    gPayloads[sel].fn
                );

            if (gPop.thread)
            {
                EnableWindow(gPop.combo,   FALSE);
                EnableWindow(gPop.launch,  FALSE);
                EnableWindow(gPop.stop,    TRUE);
            }
            return 0;
        }

        case IDC_STOP:
            StopRunning();
            return 0;

        case IDC_CONTINUE:
            StopRunning();
            gPop.result = 1;
            DestroyWindow(hwnd);
            return 0;
        }
        return 0;

    case WM_CLOSE:
        StopRunning();
        gPop.result = 0;
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static int RunPopup()
{
    const wchar_t* cls = L"HPPD";

    WNDCLASSW wc{};

    wc.lpfnWndProc   = PopupProc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = cls;

    RegisterClassW(&wc);

    const DWORD style =
        WS_OVERLAPPED |
        WS_CAPTION |
        WS_SYSMENU |
        WS_MINIMIZEBOX;

    const int cw = 460;
    const int ch = 340;

    RECT rc{ 0, 0, cw, ch };

    AdjustWindowRect(&rc, style, FALSE);

    const int ww = rc.right - rc.left;
    const int wh = rc.bottom - rc.top;

    const int sw = GetSystemMetrics(SM_CXSCREEN);
    const int sh = GetSystemMetrics(SM_CYSCREEN);

    gPop = PopupState{};

    gPop.hwnd =
        CreateWindowExW(
            0,
            cls,
            L"HPPD Visual Demo",
            style,
            (sw - ww) / 2,
            (sh - wh) / 2,
            ww,
            wh,
            nullptr,
            nullptr,
            wc.hInstance,
            nullptr
        );

    if (!gPop.hwnd)
        return 0;

    // Warning text.
    CreateWindowExW(
        0,
        L"STATIC",
        L"This is a malicious software, which aims to be a demo.\r\n\r\n"
        L"It does modify your disk, crash Windows, "
        L"change personal files, or play audio.\r\n\r\n"
        L"From now on, its your choice if you want to execute it.\r\n\r\n"
        L"ARE YOU SURE YOU WANT TO PERMANENTLY DESTROY YOUR DEVICE?\r\n\r\n",
        //L"Press ESC to skip the current effect.\r\n"
        //L"Press Ctrl+F2 to toggle the payload/time overlay.\r\n"
        //L"Press Ctrl+F12 to open the payload selector.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        14, 10, cw - 28, 110,
        gPop.hwnd,
        nullptr,
        wc.hInstance,
        nullptr
    );

    // Drop-down list of payloads.
    gPop.combo =
        CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"COMBOBOX",
            nullptr,
            WS_CHILD |
            WS_TABSTOP |
            CBS_DROPDOWNLIST,
            14, 130, cw - 28, 200,
            gPop.hwnd,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_COMBO)),
            wc.hInstance,
            nullptr
        );

    for (int i = 0; i < gPayloadCount; ++i)
    {
        SendMessageW(
            gPop.combo,
            CB_ADDSTRING,
            0,
            reinterpret_cast<LPARAM>(
                gPayloads[i].name
                )
        );
    }

    // Selected payload nickname.
    gPop.nick =
        CreateWindowExW(
            0,
            L"STATIC",
            nullptr,
            WS_CHILD | SS_LEFT,
            14, 162, cw - 28, 18,
            gPop.hwnd,
            nullptr,
            wc.hInstance,
            nullptr
        );

    // Launch (bottom-left), Continue (center), STOP (bottom-right).
    gPop.launch =
        CreateWindowExW(
            0,
            L"BUTTON",
            L"Launch",
            WS_CHILD |
            WS_VISIBLE |
            WS_TABSTOP |
            BS_PUSHBUTTON,
            14, 296, 120, 30,
            gPop.hwnd,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_LAUNCH)),
            wc.hInstance,
            nullptr
        );

    gPop.stop =
        CreateWindowExW(
            0,
            L"BUTTON",
            L"STOP",
            WS_CHILD |
            WS_VISIBLE |
            WS_TABSTOP |
            BS_PUSHBUTTON,
            cw - 134, 296, 120, 30,
            gPop.hwnd,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_STOP)),
            wc.hInstance,
            nullptr
        );

    CreateWindowExW(
        0,
        L"BUTTON",
        L"Continue",
        WS_CHILD |
        WS_VISIBLE |
        WS_TABSTOP |
        BS_DEFPUSHBUTTON,
        (cw - 120) / 2, 296, 120, 30,
        gPop.hwnd,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CONTINUE)),
        wc.hInstance,
        nullptr
    );

    // Selector is hidden until Ctrl+F12.
    ShowWindow(gPop.combo,   SW_HIDE);
    ShowWindow(gPop.nick,    SW_HIDE);
    ShowWindow(gPop.launch,  SW_HIDE);
    ShowWindow(gPop.stop,    SW_HIDE);

    ShowWindow(gPop.hwnd, SW_SHOW);
    UpdateWindow(gPop.hwnd);

    gFxStop = false;

    MSG msg;

    for (;;)
    {
        // Ctrl+F12 toggle works no matter which control
        // currently has focus.
        const bool ctrl =
            (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

        const bool f12 =
            (GetAsyncKeyState(VK_F12) & 0x8000) != 0;

        if (ctrl && f12 && !gPop.f12WasDown)
            ToggleSelector();

        gPop.f12WasDown = ctrl && f12;

        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (!IsDialogMessageW(gPop.hwnd, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }

        if (msg.message == WM_QUIT)
            break;

        Sleep(15);
    }

    StopRunning();

    UnregisterClassW(cls, wc.hInstance);

    return gPop.result;
}

// ============================================================
// NON-VM WARNING
//
// Informative only: when the VM checks report no hypervisor /
// VM vendor on the host, show a read-only notice and exit after
// 10 seconds. No code input, no confirmation flow, no
// destructive action — just "this demo targets a VM".
// ============================================================

namespace
{
    const int IDC_VMWARN_CLOSE = 1101;

    struct VmWarnState
    {
        HWND hwnd   = nullptr;
        HWND left   = nullptr; // seconds remaining
        HWND ok     = nullptr;
        int  remain = 10;
    };

    VmWarnState gVmWarn;
}

static void VmWarnTick()
{
    if (gVmWarn.remain <= 0)
    {
        DestroyWindow(gVmWarn.hwnd);
        return;
    }

    wchar_t buf[96];

    wsprintfW(
        buf,
        L"This window closes automatically in %d second%s.",
        gVmWarn.remain,
        gVmWarn.remain == 1 ? L"" : L"s"
    );

    SetWindowTextW(gVmWarn.left, buf);

    --gVmWarn.remain;
}

static LRESULT CALLBACK VmWarnProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (msg)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_VMWARN_CLOSE)
        {
            DestroyWindow(hwnd);
            return 0;
        }
        return 0;

    case WM_TIMER:
        VmWarnTick();
        return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// Shows the notice and runs its message loop until it closes
// (either the 10s countdown or the Close button).
static void ShowVmWarning()
{
    const wchar_t* cls = L"HPPD_VMWARN";

    WNDCLASSW wc{};

    wc.lpfnWndProc   = VmWarnProc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = cls;

    RegisterClassW(&wc);

    const DWORD style =
        WS_OVERLAPPED |
        WS_CAPTION |
        WS_SYSMENU;

    const int cw = 480;
    const int ch = 200;

    RECT rc{ 0, 0, cw, ch };

    AdjustWindowRect(&rc, style, FALSE);

    const int ww = rc.right - rc.left;
    const int wh = rc.bottom - rc.top;

    const int sw = GetSystemMetrics(SM_CXSCREEN);
    const int sh = GetSystemMetrics(SM_CYSCREEN);

    gVmWarn = VmWarnState{};

    gVmWarn.hwnd =
        CreateWindowExW(
            0,
            cls,
            L"PROCEED CAREFULLY",
            style,
            (sw - ww) / 2,
            (sh - wh) / 2,
            ww,
            wh,
            nullptr,
            nullptr,
            wc.hInstance,
            nullptr
        );

    if (!gVmWarn.hwnd)
    {
        UnregisterClassW(cls, wc.hInstance);
        return;
    }

    CreateWindowExW(
        0,
        L"STATIC",
        L"The built-in VM checks returned false, which means that "
        L"you're NOT running this on a virtual machine.\r\n\r\n"
        L"This program will close automatically in 10 seconds "
        L"(main process and other threads).\r\n\r\n"
        L"No changes are made to your system.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        14, 10, cw - 28, 90,
        gVmWarn.hwnd,
        nullptr,
        wc.hInstance,
        nullptr
    );

    gVmWarn.left =
        CreateWindowExW(
            0,
            L"STATIC",
            nullptr,
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            14, 104, cw - 28, 18,
            gVmWarn.hwnd,
            nullptr,
            wc.hInstance,
            nullptr
        );

    gVmWarn.ok =
        CreateWindowExW(
            0,
            L"BUTTON",
            L"Close",
            WS_CHILD |
            WS_VISIBLE |
            WS_TABSTOP |
            BS_DEFPUSHBUTTON,
            (cw - 120) / 2, 142, 120, 30,
            gVmWarn.hwnd,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_VMWARN_CLOSE)),
            wc.hInstance,
            nullptr
        );

    ShowWindow(gVmWarn.hwnd, SW_SHOW);
    UpdateWindow(gVmWarn.hwnd);

    VmWarnTick(); // prints "10 seconds"

    SetTimer(
        gVmWarn.hwnd,
        1,
        1000,
        nullptr
    );

    MSG msg;

    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        if (!IsDialogMessageW(gVmWarn.hwnd, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    KillTimer(gVmWarn.hwnd, 1);

    UnregisterClassW(cls, wc.hInstance);
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    gSeed =
        static_cast<uint32_t>(
            GetTickCount64()
        ) ^ static_cast<uint32_t>(__rdtsc());

    printf("Global SEED: %u\n", gSeed);
    fflush(stdout);

    // Easter eggs: date-based, desktop-based, system-based.
    EasterDates();
    EasterDesk();
    EasterSys();

    // VM gate: this demo targets a virtual machine. On bare
    // metal (or when detection is inconclusive) show a read-only
    // notice and exit without running any payload.
    if (!IsVirtualMachine())
    {
        ShowVmWarning();
        return 0;
    }

    // Warning popup. Ctrl+F12 opens the payload selector;
    // returns 1 to run the full demo, 0 to exit.
    if (RunPopup() != 1)
        return 0;

    StartOverlay();

    // Default soundtrack: bytebeat plays throughout the whole
    // payload. On March 29 a special payload (64) plays the
    // embedded XM song instead, so the bytebeat stays off to
    // avoid two threads grabbing the same waveOut device.
    SYSTEMTIME now;
    GetLocalTime(&now);

    const bool easterToday =
        (now.wMonth == 3 && now.wDay == 29);

    Bytebeats::Reset();
    Bytebeats::Play(
        "t*(t^t>>10)|t>>2|t>>8|t&t>>4",
        2000
    );

    HANDLE sound = nullptr;

    if (!easterToday)
        sound = StartEffect(Bytebeats::BytebeatPlay);

#if HPPD_MULTI
    // -------------------------------------------------------
    // MULTI: all payloads share the screen at once. Additive
    // content-adder effects and full-screen GDI modifiers
    // overlap, producing one combined chaos.
    // -------------------------------------------------------

    HANDLE threads[65];

    gHeavySlot = CreateMutexW(nullptr, FALSE, nullptr);

    for (int i = 0; i < gPayloadCount; ++i)
    {
        OverlaySetPayload(
            gPayloads[i].name,
            gPayloads[i].ms
        );

        if (gPayloads[i].heavy)
        {
            threads[i] =
                CreateThread(
                    nullptr,
                    0,
                    SlotRunner,
                    new SlotArg{
                        gPayloads[i].fn
                    },
                    0,
                    nullptr
                );
        }
        else
        {
            threads[i] =
                StartEffect(gPayloads[i].fn);
        }
    }

    for (int i = 0; i < gPayloadCount; ++i)
        WaitEffect(threads[i]);

    CloseHandle(gHeavySlot);
    gHeavySlot = nullptr;
#else
    // -------------------------------------------------------
    // DEFAULT: one effect at a time, in order.
    // -------------------------------------------------------

    for (int i = 0; i < gPayloadCount; ++i)
    {
        OverlaySetPayload(
            gPayloads[i].name,
            gPayloads[i].ms
        );

        runEffect(gPayloads[i].fn);
    }
#endif

    // Silence and reap the soundtrack thread.
    if (sound)
    {
        Bytebeats::StopAll();
        WaitEffect(sound);
    }

    // --------------------------------------------------------
    // Final cleanup
    // --------------------------------------------------------

    RedrawWindow(
        NULL,
        NULL,
        NULL,
        RDW_INVALIDATE |
        RDW_ERASE |
        RDW_UPDATENOW |
        RDW_ALLCHILDREN
    );

    MessageBoxW(
        NULL,
        L"dead end.",
        L"HPPD",
        MB_OK |
        MB_ICONINFORMATION
    );

    return 0;
}