#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <cstdint>
#include <algorithm>

// ============================================================
// SHARED DIB BUFFER
//
// Per-pixel remap/shaders run on a temporary screen-sized DIB.
// Used by the heavy FX packs (50-63) and the light FX pack 3
// (69-72) plus the Death Month renderer. Functions live inline
// here so every payload TU sees the same helpers.
// ============================================================

namespace
{
    static int ClampI(int x, int a, int b)
    {
        return std::max(a, std::min(b, x));
    }

    static uint32_t RGBPixel(int r, int g, int b)
    {
        return
            ((uint32_t)ClampI(b, 0, 255) << 16) |
            ((uint32_t)ClampI(g, 0, 255) << 8) |
            (uint32_t)ClampI(r, 0, 255);
    }

    static void SplitRGB(
        uint32_t p,
        int& r,
        int& g,
        int& b
    )
    {
        r = p & 255;
        g = (p >> 8) & 255;
        b = (p >> 16) & 255;
    }

    struct GDIBuffer
    {
        HDC dc = nullptr;
        HBITMAP bmp = nullptr;
        HGDIOBJ old = nullptr;
        uint32_t* pixels = nullptr;
        int w = 0;
        int h = 0;
    };

    static bool CreateBuffer(
        HDC source,
        GDIBuffer& out,
        int w,
        int h
    )
    {
        out.w = w;
        out.h = h;

        out.dc = CreateCompatibleDC(source);

        if (!out.dc)
            return false;

        BITMAPINFO bi{};
        bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth = w;
        bi.bmiHeader.biHeight = -h;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        out.bmp =
            CreateDIBSection(
                source,
                &bi,
                DIB_RGB_COLORS,
                reinterpret_cast<void**>(&out.pixels),
                nullptr,
                0
            );

        if (!out.bmp || !out.pixels)
        {
            if (out.bmp)
                DeleteObject(out.bmp);

            DeleteDC(out.dc);

            out.dc = nullptr;
            out.bmp = nullptr;
            out.pixels = nullptr;

            return false;
        }

        out.old = SelectObject(out.dc, out.bmp);

        return true;
    }

    static void DestroyBuffer(GDIBuffer& b)
    {
        if (b.dc)
        {
            SelectObject(b.dc, b.old);
            DeleteDC(b.dc);
        }

        if (b.bmp)
            DeleteObject(b.bmp);

        b = {};
    }
}