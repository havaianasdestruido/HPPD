#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <cstdint>
#include <algorithm>

// ============================================================
// Safe visual-only GDI effects demo
//
// Payloads 1-24
//
// No:
// - disk writes
// - MBR modification
// - privilege escalation
// - BSOD / NtRaiseHardError
// - audio playback
//
// ESC aborts Payloads 13-24.
// Payloads 1-12 retain their original finite durations.
// ============================================================

// ============================================================
// Shared helper
// ============================================================

static inline int ClampInt(int value, int low, int high)
{
    if (value < low)
        return low;

    if (value > high)
        return high;

    return value;
}

// ============================================================
// FAST SHARED 32-BIT DIB
// Used by Payloads 13-24
// ============================================================

struct GDISurface
{
    HDC screen = nullptr;
    HDC memDC = nullptr;

    HBITMAP bitmap = nullptr;
    HGDIOBJ oldBitmap = nullptr;

    uint32_t* pixels = nullptr;

    int w = 0;
    int h = 0;

    bool Init()
    {
        screen = GetDC(nullptr);

        if (!screen)
            return false;

        w = GetSystemMetrics(SM_CXSCREEN);
        h = GetSystemMetrics(SM_CYSCREEN);

        memDC = CreateCompatibleDC(screen);

        if (!memDC)
            return false;

        BITMAPINFO bi{};

        bi.bmiHeader.biSize =
            sizeof(BITMAPINFOHEADER);

        bi.bmiHeader.biWidth = w;

        // Top-down DIB.
        bi.bmiHeader.biHeight = -h;

        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        bitmap = CreateDIBSection(
            screen,
            &bi,
            DIB_RGB_COLORS,
            reinterpret_cast<void**>(&pixels),
            nullptr,
            0
        );

        if (!bitmap || !pixels)
            return false;

        oldBitmap =
            SelectObject(
                memDC,
                bitmap
            );

        return true;
    }

    void Capture()
    {
        BitBlt(
            memDC,
            0,
            0,
            w,
            h,
            screen,
            0,
            0,
            SRCCOPY
        );
    }

    void Present()
    {
        BitBlt(
            screen,
            0,
            0,
            w,
            h,
            memDC,
            0,
            0,
            SRCCOPY
        );
    }

    void Destroy()
    {
        if (memDC && oldBitmap)
            SelectObject(
                memDC,
                oldBitmap
            );

        if (bitmap)
            DeleteObject(bitmap);

        if (memDC)
            DeleteDC(memDC);

        if (screen)
            ReleaseDC(
                nullptr,
                screen
            );

        bitmap = nullptr;
        memDC = nullptr;
        screen = nullptr;
        pixels = nullptr;
    }

    ~GDISurface()
    {
        Destroy();
    }
};

// ============================================================
// Fast helpers
// ============================================================

static inline uint8_t Clamp8(int v)
{
    if (v < 0)
        return 0;

    if (v > 255)
        return 255;

    return static_cast<uint8_t>(v);
}

static inline uint32_t PackRGB(
    int r,
    int g,
    int b
)
{
    return
        static_cast<uint32_t>(
            Clamp8(b)
            ) << 16 |
        static_cast<uint32_t>(
            Clamp8(g)
            ) << 8 |
        static_cast<uint32_t>(
            Clamp8(r)
            );
}

static inline bool FXAbort()
{
    return (
        GetAsyncKeyState(VK_ESCAPE)
        & 0x8000
        ) != 0;
}

static inline void FPS16(
    ULONGLONG& nextFrame
)
{
    nextFrame += 16;

    ULONGLONG now =
        GetTickCount64();

    if (now < nextFrame)
    {
        Sleep(
            static_cast<DWORD>(
                nextFrame - now
                )
        );
    }
    else
    {
        nextFrame = now;
    }
}

static inline bool FXTimeout(
    ULONGLONG start,
    DWORD duration
)
{
    return
        FXAbort() ||
        (
            GetTickCount64() -
            start >= duration
            );
}

// Extremely cheap xorshift RNG.
static inline uint32_t FXRand(
    uint32_t& state
)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    return state;
}

// ============================================================
// Payload declarations
// ============================================================

DWORD WINAPI GDIPayload1(LPVOID);
DWORD WINAPI GDIPayload2(LPVOID);
DWORD WINAPI GDIPayload3(LPVOID);
DWORD WINAPI GDIPayload4(LPVOID);
DWORD WINAPI GDIPayload5(LPVOID);
DWORD WINAPI GDIPayload6(LPVOID);
DWORD WINAPI GDIPayload7(LPVOID);
DWORD WINAPI GDIPayload8(LPVOID);
DWORD WINAPI GDIPayload9(LPVOID);
DWORD WINAPI GDIPayload10(LPVOID);
DWORD WINAPI GDIPayload11(LPVOID);
DWORD WINAPI GDIPayload12(LPVOID);
DWORD WINAPI GDIPayload13(LPVOID);
DWORD WINAPI GDIPayload14(LPVOID);
DWORD WINAPI GDIPayload15(LPVOID);
DWORD WINAPI GDIPayload16(LPVOID);
DWORD WINAPI GDIPayload17(LPVOID);
DWORD WINAPI GDIPayload18(LPVOID);
DWORD WINAPI GDIPayload19(LPVOID);
DWORD WINAPI GDIPayload20(LPVOID);
DWORD WINAPI GDIPayload21(LPVOID);
DWORD WINAPI GDIPayload22(LPVOID);
DWORD WINAPI GDIPayload23(LPVOID);
DWORD WINAPI GDIPayload24(LPVOID);
