#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <cstdlib>

#include "cursor.h"

#pragma comment(lib, "user32.lib")

namespace Cursor
{

namespace
{

// System cursor id for the normal arrow (missing from 7.1A winuser.h).
const DWORD kOcrNormal = 32512;

// The six cursors a user sees most.
const struct { LPCWSTR source; LPCWSTR target; } kSwaps[] =
{
	{ IDC_ARROW,       IDC_IBEAM },
	{ IDC_IBEAM,       IDC_WAIT },
	{ IDC_WAIT,        IDC_CROSS },
	{ IDC_CROSS,       IDC_HAND },
	{ IDC_HAND,        IDC_SIZEALL },
	{ IDC_SIZEALL,     IDC_ARROW },
};

bool gArmed = false;

void Arm(LPCWSTR target, HCURSOR cursor)
{
	HCURSOR copy = CopyCursor(cursor);
	if (copy)
	{
		SetSystemCursor(copy, static_cast<DWORD>(
			reinterpret_cast<UINT_PTR>(target)));
		gArmed = true;
	}
}

} // namespace

bool SwapStandard()
{
	for (const auto& swap : kSwaps)
	{
		HCURSOR src = LoadCursorW(nullptr, swap.source);
		if (src)
			Arm(swap.target, src);
	}
	return true;
}

void Restore()
{
	SystemParametersInfoW(SPI_SETCURSORS, 0, nullptr, 0);
	gArmed = false;
}

bool Armed()
{
	return gArmed;
}

void Chaos(DWORD ms)
{
	// For each tick pick a random cursor from the scramble list and
	// drop it on OCR_NORMAL so the pointer keeps changing shape.
	static const LPCWSTR kGallery[] =
	{
		IDC_HAND, IDC_WAIT, IDC_HELP, IDC_CROSS, IDC_UPARROW,
		IDC_IBEAM, IDC_SIZEALL, IDC_NO,
	};

	srand(GetTickCount());

	for (DWORD t = 0; t < ms; t += 300)
	{
		HCURSOR src = LoadCursorW(nullptr, kGallery[rand() % 8]);
		if (src)
		{
HCURSOR copy = CopyCursor(src);
				if (copy)
				{
					SetSystemCursor(copy, kOcrNormal);
					gArmed = true;
				}
		}
		Sleep(300);
	}

	Restore();
}

} // namespace Cursor