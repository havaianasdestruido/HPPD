#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <cstdint>
#include <algorithm>

#include "utils/colors.h"
#include "utils/seed.h"

// ============================================================
// HPPD - effects demo
//
// Payloads 1-63.
//
// 1-12  original visual GDI effects (finite duration).
// 13-24 fast visual GDI effects (5s, ESC aborts).
// 25-49 mixed visual + window/cursor/process manipulation
//        ported from a public bytebeat/GDI-effects dump.
//        No disk writes, no MBR, no privilege escalation,
//        no BSOD, no audio playback.
// 50-57 GDI demoscene FX pack (per-pixel remap/shaders).
// 58-63 GDI demoscene FX pack 2 (echo stack, interference
//        fringes, pixel sort, Julia set, flow field,
//        reaction-diffusion).
//
// ESC aborts every payload (1-63); the popup's STOP button
// sets the same gFxStop latch.
// Payloads 1-12 retain their original finite durations
// unless aborted early.
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

// Shifts a packed BGR pixel through HSL space using
// Colors::rgb2hsl / Colors::hsl2rgb.
static inline uint32_t ShiftHSL(
    uint32_t px,
    FLOAT dHue,
    FLOAT dSat,
    FLOAT dLight
)
{
    RGBQUAD rgb{};

    rgb.rgbRed =
        px & 255;

    rgb.rgbGreen =
        (px >> 8) & 255;

    rgb.rgbBlue =
        (px >> 16) & 255;

    HSL hsl =
        Colors::rgb2hsl(rgb);

    hsl.h += dHue;
    hsl.s += dSat;
    hsl.l += dLight;

    if (hsl.h < 0.f)
        hsl.h += 1.f;

    if (hsl.h >= 1.f)
        hsl.h -= 1.f;

    if (hsl.s < 0.f)
        hsl.s = 0.f;

    if (hsl.s > 1.f)
        hsl.s = 1.f;

    if (hsl.l < 0.f)
        hsl.l = 0.f;

    if (hsl.l > 1.f)
        hsl.l = 1.f;

    RGBQUAD out =
        Colors::hsl2rgb(hsl);

    return PackRGB(
        out.rgbRed,
        out.rgbGreen,
        out.rgbBlue
    );
}

// Global "stop payload" latch, set by the popup's STOP button.
extern volatile bool gFxStop;

static inline bool FXAbort()
{
    return (
        GetAsyncKeyState(VK_ESCAPE)
        & 0x8000
        ) != 0 ||
        gFxStop;
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

// i prob should make this more organized but who cares

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
DWORD WINAPI GDIPayload25(LPVOID);
DWORD WINAPI GDIPayload26(LPVOID);
DWORD WINAPI GDIPayload27(LPVOID);
DWORD WINAPI GDIPayload28(LPVOID);
DWORD WINAPI GDIPayload29(LPVOID);
DWORD WINAPI GDIPayload30(LPVOID);
DWORD WINAPI GDIPayload31(LPVOID);
DWORD WINAPI GDIPayload32(LPVOID);
DWORD WINAPI GDIPayload33(LPVOID);
DWORD WINAPI GDIPayload34(LPVOID);
DWORD WINAPI GDIPayload35(LPVOID);
DWORD WINAPI GDIPayload36(LPVOID);
DWORD WINAPI GDIPayload37(LPVOID);
DWORD WINAPI GDIPayload38(LPVOID);
DWORD WINAPI GDIPayload39(LPVOID);
DWORD WINAPI GDIPayload40(LPVOID);
DWORD WINAPI GDIPayload41(LPVOID);
DWORD WINAPI GDIPayload42(LPVOID);
DWORD WINAPI GDIPayload43(LPVOID);
DWORD WINAPI GDIPayload44(LPVOID);
DWORD WINAPI GDIPayload45(LPVOID);
DWORD WINAPI GDIPayload46(LPVOID);
DWORD WINAPI GDIPayload47(LPVOID);
DWORD WINAPI GDIPayload48(LPVOID);
DWORD WINAPI GDIPayload49(LPVOID);
DWORD WINAPI GDIPayload50(LPVOID);
DWORD WINAPI GDIPayload51(LPVOID);
DWORD WINAPI GDIPayload52(LPVOID);
DWORD WINAPI GDIPayload53(LPVOID);
DWORD WINAPI GDIPayload54(LPVOID);
DWORD WINAPI GDIPayload55(LPVOID);
DWORD WINAPI GDIPayload56(LPVOID);
DWORD WINAPI GDIPayload57(LPVOID);
DWORD WINAPI GDIPayload58(LPVOID);
DWORD WINAPI GDIPayload59(LPVOID);
DWORD WINAPI GDIPayload60(LPVOID);
DWORD WINAPI GDIPayload61(LPVOID);
DWORD WINAPI GDIPayload62(LPVOID);
DWORD WINAPI GDIPayload63(LPVOID);
DWORD WINAPI GDIPayload64(LPVOID);

// ============================================================
// DEATH MONTH (DECEMBER)
//
// Every December day, from midnight to 00:01, the demo runs a
// dedicated payload for that exact day. gDeathMonth[] maps each
// day 1..31 to a payload. Adding a future day is easy:
//   1. write DWORD WINAPI Fn(LPVOID) in payloads.cpp
//   2. add one row { day, L"label", 5000, Fn } to gDeathMonth[]
// Days without a row (or outside the window) skip silently.
// ============================================================

struct DeathMonthEntry
{
    int day;                    // 1..31 (December)
    const wchar_t* name;        // overlay label
    DWORD ms;                   // overlay duration
    LPTHREAD_START_ROUTINE fn;  // payload to run
};

DWORD WINAPI GDIPDeath01(LPVOID);
DWORD WINAPI GDIPDeath02(LPVOID);
DWORD WINAPI GDIPDeath03(LPVOID);
DWORD WINAPI GDIPDeath04(LPVOID);
DWORD WINAPI GDIPDeath05(LPVOID);
DWORD WINAPI GDIPDeath06(LPVOID);
DWORD WINAPI GDIPDeath07(LPVOID);
DWORD WINAPI GDIPDeath08(LPVOID);
DWORD WINAPI GDIPDeath09(LPVOID);
DWORD WINAPI GDIPDeath10(LPVOID);
DWORD WINAPI GDIPDeath11(LPVOID);
DWORD WINAPI GDIPDeath12(LPVOID);
DWORD WINAPI GDIPDeath13(LPVOID);
DWORD WINAPI GDIPDeath14(LPVOID);
DWORD WINAPI GDIPDeath15(LPVOID);
DWORD WINAPI GDIPDeath16(LPVOID);
DWORD WINAPI GDIPDeath17(LPVOID);
DWORD WINAPI GDIPDeath18(LPVOID);
DWORD WINAPI GDIPDeath19(LPVOID);
DWORD WINAPI GDIPDeath20(LPVOID);
DWORD WINAPI GDIPDeath21(LPVOID);
DWORD WINAPI GDIPDeath22(LPVOID);
DWORD WINAPI GDIPDeath23(LPVOID);
DWORD WINAPI GDIPDeath24(LPVOID);
DWORD WINAPI GDIPDeath25(LPVOID);
DWORD WINAPI GDIPDeath26(LPVOID);
DWORD WINAPI GDIPDeath27(LPVOID);
DWORD WINAPI GDIPDeath28(LPVOID);
DWORD WINAPI GDIPDeath29(LPVOID);
DWORD WINAPI GDIPDeath30(LPVOID);
DWORD WINAPI GDIPDeath31(LPVOID);

extern const DeathMonthEntry gDeathMonth[];
extern const int gDeathMonthCount;
const DeathMonthEntry* DeathMonthToday(); // active entry or nullptr
const char* DeathMonthName();             // banner label when active
