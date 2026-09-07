#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <cstdio>
#include <intrin.h>

#include "utils.h"
#include "payloads.h"
#include "seed.h"

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
// ESC aborts Payloads 13-49.
// Payloads 1-12 retain their original finite durations.
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
// PAYLOAD CATALOG (13-24 run; 1-12 are kept commented out
// for compatibility with the default build).
// ============================================================

struct PayloadEntry
{
    const wchar_t* name;
    DWORD ms;
    LPTHREAD_START_ROUTINE fn;
};

static PayloadEntry gPayloads[] =
{
    { L"13 - Chromatic Plasma",        5000, GDIPayload13 },
    { L"14 - RGB Displacement",        5000, GDIPayload14 },
    { L"15 - Feedback Tunnel",         5000, GDIPayload15 },
    { L"16 - Fractured Screen",        5000, GDIPayload16 },
    { L"17 - Neon Edge Inversion",     5000, GDIPayload17 },
    { L"18 - Liquid Ripple",           5000, GDIPayload18 },
    { L"19 - Quantum Spiral",          5000, GDIPayload19 },
    { L"20 - Color Worm",              5000, GDIPayload20 },
    { L"21 - Radial Shockwave",        5000, GDIPayload21 },
    { L"22 - RGB Void",                5000, GDIPayload22 },
    { L"23 - Retro CRT",               5000, GDIPayload23 },
    { L"24 - Absolute GDI Chaos",      5000, GDIPayload24 },
    { L"25 - Dark",                    5000, GDIPayload25 },
    { L"26 - Cursors",                 5000, GDIPayload26 },
    { L"27 - Programs",                5000, GDIPayload27 },
    { L"28 - Buttons",                 5000, GDIPayload28 },
    { L"29 - RGBCircle",               5000, GDIPayload29 },
    { L"30 - IconWave",                5000, GDIPayload30 },
    { L"31 - Sinewave",                5000, GDIPayload31 },
    { L"32 - Shake",                   5000, GDIPayload32 },
    { L"33 - Dark2",                   5000, GDIPayload33 },
    { L"34 - Melter",                  5000, GDIPayload34 },
    { L"35 - Trails",                  5000, GDIPayload35 },
    { L"36 - ResizeWindow",            5000, GDIPayload36 },
    { L"37 - MoveWindow",              5000, GDIPayload37 },
    { L"38 - Melter2",                 5000, GDIPayload38 },
    { L"39 - WindowMax",               5000, GDIPayload39 },
    { L"40 - Rotate",                  5000, GDIPayload40 },
    { L"41 - Rainbow",                 5000, GDIPayload41 },
    { L"42 - RandomCur",               5000, GDIPayload42 },
    { L"43 - Melter3",                 5000, GDIPayload43 },
    { L"44 - Colors",                  5000, GDIPayload44 },
    { L"45 - Flip",                    5000, GDIPayload45 },
    { L"46 - CircleSquare",            5000, GDIPayload46 },
    { L"47 - Bright",                  5000, GDIPayload47 },
    { L"48 - ExtremeRotation",         5000, GDIPayload48 },
    { L"49 - Cubes",                   5000, GDIPayload49 },
};

static const int gPayloadCount =
    static_cast<int>(
        sizeof(gPayloads) /
        sizeof(gPayloads[0])
    );

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
    const wchar_t* cls = L"BromineVisualDemoPopup";

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
            L"Bromine Visual Demo",
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
        L"This is a visual-effects demo.\r\n\r\n"
        L"It does not modify your disk, crash Windows, "
        L"change privileges, or play audio.\r\n\r\n"
        L"Press ESC to skip the current effect.\r\n"
        L"Press Ctrl+F2 to toggle the payload/time overlay.\r\n"
        L"Press Ctrl+F12 to open the payload selector.",
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

    // Warning popup. Ctrl+F12 opens the payload selector;
    // returns 1 to run the full demo, 0 to exit.
    if (RunPopup() != 1)
        return 0;

    StartOverlay();

#if HPPD_MULTI
    // -------------------------------------------------------
    // MULTI: all payloads share the screen at once. Additive
    // content-adder effects and full-screen GDI modifiers
    // overlap, producing one combined chaos.
    // -------------------------------------------------------

    HANDLE threads[64];

    for (int i = 0; i < gPayloadCount; ++i)
    {
        OverlaySetPayload(
            gPayloads[i].name,
            gPayloads[i].ms
        );

        threads[i] =
            StartEffect(gPayloads[i].fn);
    }

    for (int i = 0; i < gPayloadCount; ++i)
        WaitEffect(threads[i]);
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
        L"Visual demo finished.",
        L"Bromine Visual Demo",
        MB_OK |
        MB_ICONINFORMATION
    );

    return 0;
}