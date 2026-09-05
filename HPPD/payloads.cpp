#include "payloads.h"
#include "utils.h"

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

// ============================================================
// PAYLOAD 1
// Screen feedback / microscopic distortion
// ============================================================

DWORD WINAPI GDIPayload1(LPVOID)
{
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

        Sleep(rand() % 25);

        if (rand() % 25 == 24)
        {
            RedrawWindow(
                NULL,
                NULL,
                NULL,
                RDW_INVALIDATE |
                RDW_UPDATENOW |
                RDW_ALLCHILDREN
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
// PAYLOAD 2
// Pixelation / recursive zoom
// ============================================================

DWORD WINAPI GDIPayload2(LPVOID)
{
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
        SetStretchBltMode(dcCopy, COLORONCOLOR);
        SetStretchBltMode(dc, COLORONCOLOR);

        StretchBlt(
            dcCopy,
            0,
            0,
            std::max(1, w / 3),
            std::max(1, h / 3),
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
            std::max(1, w / 3),
            std::max(1, h / 3),
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
    HDC dc = GetDC(NULL);

    if (!dc)
        return 0;

    const int w = GetSystemMetrics(SM_CXSCREEN);
    const int h = GetSystemMetrics(SM_CYSCREEN);

    HICON errorIcon = LoadIcon(NULL, IDI_ERROR);

    for (int i = 0; i < 300; ++i)
    {
        if (errorIcon)
        {
            DrawIcon(
                dc,
                rand() % w,
                rand() % h,
                errorIcon
            );
        }

        Sleep(50);

        if (i % 25 == 24)
        {
            RedrawWindow(
                NULL,
                NULL,
                NULL,
                RDW_INVALIDATE |
                RDW_UPDATENOW |
                RDW_ALLCHILDREN
            );
        }
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
    HDC dc = GetDC(NULL);

    if (!dc)
        return 0;

    const int w = GetSystemMetrics(SM_CXSCREEN);
    const int h = GetSystemMetrics(SM_CYSCREEN);

    for (int x = 0; x <= w; x += 16)
    {
        for (int y = 0; y <= h; y += 16)
        {
            PatBlt(
                dc,
                0,
                0,
                w,
                h,
                PATINVERT
            );

            Sleep(rand() % 20);

            if ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0)
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
        BitBlt(
            dcCopy,
            rand() % 10,
            rand() % 12,
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

        Sleep(rand() % 25);
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
                            (x + t * 4) * 0.025
                        ) * 32
                        );

                pixels[i] =
                    ShiftHSL(
                        pixels[i],
                        wave / 255.0f,
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
                        t * std::sin(x / 16.0) +
                        t +
                        t * std::sin(y / 8.0) +
                        t +
                        t * std::sin((x + y) / 16.0) +
                        t +
                        t * std::sin(
                            std::sqrt(
                                static_cast<double>(
                                    x * x + y * y
                                    )
                            ) / 8.0
                        )
                        ) / 4.0;

                int wave =
                    static_cast<int>(
                        std::sin(fx * 0.02) * 50.0
                        );

                pixels[i] =
                    ShiftHSL(
                        pixels[i],
                        -wave / 255.0f,
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
                    x * fractalX - 2.0;

                double cy =
                    y * fractalY - 0.95;

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
        int j = 1 + rand() % 12;

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
                        std::sin(fx * 0.025f) * 45.0f
                        );

                pixels[i] =
                    ShiftHSL(
                        pixels[i],
                        -wave / 255.0f,
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

        if (rand() % 25 == 24)
        {
            RedrawWindow(
                NULL,
                NULL,
                NULL,
                RDW_INVALIDATE |
                RDW_UPDATENOW |
                RDW_ALLCHILDREN
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
// PAYLOAD 10
// PATINVERT + color pulse
// ============================================================

DWORD WINAPI GDIPayload10(LPVOID)
{
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
        HBRUSH brush = CreateSolidBrush(
            RGB(
                rand() % 255,
                rand() % 255,
                rand() % 255
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

        Sleep(50);
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

    int intensity = 15;

    for (int frame = 0; frame < 300; ++frame)
    {
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
            std::max(1, w / 4),
            std::max(1, h / 4),
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

                int channel = rand() % 3;
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
            std::max(1, w / 4),
            std::max(1, h / 4),
            SRCCOPY
        );

        Sleep(25);
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
        int fx = rand() % h;

        if (rand() % 3 == 2)
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

        Sleep(10);
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
                (y * 3 + t * 5)
                & 1023;

            for (int x = 0; x < s.w; ++x)
            {
                int i =
                    y * s.w + x;

                int rWave =
                    sine[
                        (x + t * 4)
                            & 1023
                    ];

                int gWave =
                    sine[
                        (x * 2 +
                            y * 3 +
                            t * 2)
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
                            / 3.0f / 255.0f,
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
                        (y + t) * 0.035
                    ) *
                    32.0
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
                    ((x + wave) & 31)
                    - 16;

                int gShift =
                    ((x - wave) & 31)
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
                    t * 0.07
                ) *
                35.0
                );

        int zoom =
            static_cast<int>(
                std::sin(
                    t * 0.035
                ) *
                35.0
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
    GDISurface s;

    if (!s.Init())
        return 0;

    uint32_t rng =
        0xA53C91EF;

    ULONGLONG start =
        GetTickCount64();

    ULONGLONG nextFrame = start;

    while (!FXTimeout(start, 5000))
    {
        s.Capture();

        for (int i = 0; i < 45; ++i)
        {
            int y =
                FXRand(rng) %
                s.h;

            int height =
                2 +
                FXRand(rng) % 32;

            int offset =
                static_cast<int>(
                    FXRand(rng) % 180
                    ) - 90;

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
                            (x + y + t)
                            * 0.025
                        ) *
                        30.0
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

        constexpr int slice = 6;

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
                        t * 0.12
                    ) *
                    45.0
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
                    t * 0.04
                ) *
                100.0
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
                    (y * 3 + t * 4)
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
                            t * 7
                            ) & 2047
                    ];

                p[i] =
                    ShiftHSL(
                        p[i],
                        wave / 255.0f,
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
            t * 0.06;

        int radius =
            static_cast<int>(
                (
                    std::sin(phase) +
                    1.0
                    ) *
                0.5 *
                (w / 2)
                );

        int distortion =
            static_cast<int>(
                std::sin(
                    phase * 2.0
                ) *
                55.0
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
                            t * 8
                            ) & 2047
                    ];

                p[i] =
                    ShiftHSL(
                        p[i],
                        wave / 255.0f / 2.0f,
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
                            t * 0.08
                        ) *
                        16
                        );

                p[i] =
                    ShiftHSL(
                        p[i],
                        aberration / 255.0f,
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
        0x91827364;

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
                FXRand(rng) % 50;

            int offset =
                static_cast<int>(
                    FXRand(rng) % 250
                    ) -
                125;

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
                        frame * 0.05
                    ) *
                    100.0
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
