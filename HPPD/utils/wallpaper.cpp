#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <cstdio>
#include <vector>

#include "wallpaper.h"

namespace Wallpaper
{

namespace
{

bool gArmed = false;
wchar_t gOriginal[MAX_PATH] = L"";
wchar_t gInverted[MAX_PATH] = L"";

#pragma pack(push, 1)
struct BmpFileHeader
{
	WORD   bfType;
	DWORD  bfSize;
	WORD   bfReserved1;
	WORD   bfReserved2;
	DWORD  bfOffBits;
};

struct BmpInfoHeader
{
	DWORD  biSize;
	LONG   biWidth;
	LONG   biHeight;
	WORD   biPlanes;
	WORD   biBitCount;
	DWORD  biCompression;
	DWORD  biSizeImage;
	LONG   biXPelsPerMeter;
	LONG   biYPelsPerMeter;
	DWORD  biClrUsed;
	DWORD  biClrImportant;
};
#pragma pack(pop)

bool WriteBmp(const wchar_t* path, int w, int h, const void* bits, DWORD bitsSize)
{
	HANDLE f = CreateFileW(path, GENERIC_WRITE, 0, nullptr,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (f == INVALID_HANDLE_VALUE)
		return false;

	BmpFileHeader fh{};
	fh.bfType = 0x4D42; // 'BM'
	fh.bfOffBits = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
	fh.bfSize = fh.bfOffBits + bitsSize;

	BmpInfoHeader ih{};
	ih.biSize = sizeof(BmpInfoHeader);
	ih.biWidth = w;
	ih.biHeight = h;
	ih.biPlanes = 1;
	ih.biBitCount = 32;
	ih.biCompression = BI_RGB;
	ih.biSizeImage = bitsSize;

	DWORD written = 0;
	bool ok = WriteFile(f, &fh, sizeof(fh), &written, nullptr) &&
		WriteFile(f, &ih, sizeof(ih), &written, nullptr) &&
		WriteFile(f, bits, bitsSize, &written, nullptr);

	CloseHandle(f);
	return ok;
}

bool CaptureInvertedBitmap(const wchar_t* outPath)
{
	const int w = GetSystemMetrics(SM_CXSCREEN);
	const int h = GetSystemMetrics(SM_CYSCREEN);
	if (w <= 0 || h <= 0)
		return false;

	HDC screen = GetDC(nullptr);
	HDC mem = CreateCompatibleDC(screen);

	HBITMAP bmp = CreateCompatibleBitmap(screen, w, h);
	HGDIOBJ old = SelectObject(mem, bmp);

	// NOTSRCCOPY = invert the captured pixels in place.
	BitBlt(mem, 0, 0, w, h, screen, 0, 0, NOTSRCCOPY);

	BITMAPINFO bi{};
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = w;
	bi.bmiHeader.biHeight = -h; // top-down
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	const DWORD rowSize = static_cast<DWORD>(w) * 4;
	const DWORD size = rowSize * static_cast<DWORD>(h);

	std::vector<unsigned char> pixels(size);
	const int lines = GetDIBits(mem, bmp, 0, h, pixels.data(), &bi, DIB_RGB_COLORS);

	SelectObject(mem, old);
	DeleteObject(bmp);
	DeleteDC(mem);
	ReleaseDC(nullptr, screen);

	if (lines == 0)
		return false;

	return WriteBmp(outPath, w, h, pixels.data(), size);
}

} // namespace

bool Invert()
{
	// Remember the original wallpaper just once.
	if (!gArmed)
	{
		SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, gOriginal, 0);

		wchar_t temp[MAX_PATH] = L"";
		GetTempPathW(MAX_PATH, temp);
		wsprintfW(gInverted, L"%shppd_invert.bmp", temp);
	}

	if (!CaptureInvertedBitmap(gInverted))
		return false;

	// No SPIF_UPDATEINIFILE: do not persist the gag across reboot.
	const BOOL ok = SystemParametersInfoW(
		SPI_SETDESKWALLPAPER, 0, gInverted, 0);

	if (ok)
		gArmed = true;

	return ok != FALSE;
}

bool InvertAndRestore(DWORD holdMs)
{
	if (!Invert())
		return false;

	Sleep(holdMs);
	return Restore();
}

bool Restore()
{
	if (!gArmed)
		return false;

	// An empty original path means "no wallpaper" -> pass an empty
	// string, which the shell treats as a blank desktop.
	SystemParametersInfoW(
		SPI_SETDESKWALLPAPER, 0,
		gOriginal[0] ? gOriginal : L"", 0);

	DeleteFileW(gInverted);
	gArmed = false;
	return true;
}

bool Armed()
{
	return gArmed;
}

} // namespace Wallpaper