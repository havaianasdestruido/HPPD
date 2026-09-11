#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

// The hook needs the keyboard low-level structures; the standard
// keyboard hooks pull in enough of winuser even with lean.
#include <windows.h>
#include <cstdlib>
#include <cstdio>

#include "keyboard.h"

namespace Keyboard
{

// ------------------------------------------------------------
// Physical key rows, left to right. VK_ names track POSITIONS,
// not characters, so "neighbor" logic is layout aware.
// ------------------------------------------------------------

namespace
{

static HHOOK gHook = nullptr;
static volatile LONG gCount = 0;
static volatile LONG gOneInN = 100;

static const int kRowDigits[] =
{
	VK_OEM_3,              // ` ~
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	VK_OEM_MINUS,          // - _
	VK_OEM_PLUS,           // = +
};

static const int kRowQwerty[] =
{
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
	VK_OEM_4,              // [ {
	VK_OEM_6,              // ] }
	VK_OEM_5,              // \ |
};

static const int kRowAsdf[] =
{
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
	VK_OEM_1,              // ; :
	VK_OEM_7,              // ' "
};

static const int kRowZxcv[] =
{
	'Z', 'X', 'C', 'V', 'B', 'N', 'M',
	VK_OEM_COMMA,          // , <
	VK_OEM_PERIOD,         // . >
	VK_OEM_2,              // / ?
};

static const int* const kRows[] = { kRowDigits, kRowQwerty, kRowAsdf, kRowZxcv };
static const int kRowCounts[] =
{
	static_cast<int>(sizeof(kRowDigits) / sizeof(kRowDigits[0])),
	static_cast<int>(sizeof(kRowQwerty) / sizeof(kRowQwerty[0])),
	static_cast<int>(sizeof(kRowAsdf) / sizeof(kRowAsdf[0])),
	static_cast<int>(sizeof(kRowZxcv) / sizeof(kRowZxcv[0])),
};

// Returns the index of `vk` in its row, or -1 when the key is
// not a candidate (excludes F1-F24, navigation, modifiers, ...).
static int FindRowIndex(int vk)
{
	for (int r = 0; r < 4; ++r)
	{
		for (int i = 0; i < kRowCounts[r]; ++i)
		{
			if (kRows[r][i] == vk)
				return i;
		}
	}
	return -1;
}

static int Neighbor(int vk, int dir)
{
	for (int r = 0; r < 4; ++r)
	{
		for (int i = 0; i < kRowCounts[r]; ++i)
		{
			if (kRows[r][i] == vk)
			{
				const int j = i + dir;
				if (j >= 0 && j < kRowCounts[r])
					return kRows[r][j]; // no wrap-around
				return vk;
			}
		}
	}
	return vk;
}

LRESULT CALLBACK MistypeProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION &&
		(wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
	{
		auto* k = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		const int vk = static_cast<int>(k->vkCode);

		if (FindRowIndex(vk) >= 0)
		{
			const LONG n = gOneInN;
			if (n > 0 && (rand() % n) == 0)
			{
				const int dir = (rand() & 1) ? 1 : -1;
				const int nbr = Neighbor(vk, dir);
				if (nbr != vk)
				{
					InterlockedIncrement(&gCount);
					k->vkCode = static_cast<DWORD>(nbr);
					k->scanCode = 0;
					k->flags     = 0;
				}
			}
		}
	}

	return CallNextHookEx(gHook, nCode, wParam, lParam);
}

} // namespace

bool InstallMistypeHook()
{
	if (gHook)
		return true;

	srand(GetTickCount() ^ (GetTickCount() << 16));

	gHook = SetWindowsHookExW(
		WH_KEYBOARD_LL, MistypeProc, GetModuleHandleW(nullptr), 0);
	return gHook != nullptr;
}

void UninstallMistypeHook()
{
	if (!gHook)
		return;

	UnhookWindowsHookEx(gHook);
	gHook = nullptr;
}

bool IsMistypeHookInstalled()
{
	return gHook != nullptr;
}

void SetMistypeChance(int oneInN)
{
	if (oneInN <= 0)
	{
		InterlockedExchange(&gOneInN, 0); // never
		return;
	}
	if (oneInN > 10000)
		oneInN = 10000;

	InterlockedExchange(&gOneInN, oneInN);
}

int MistypeCount()
{
	return static_cast<int>(InterlockedExchangeAdd(&gCount, 0));
}

void MistypeUntil(DWORD ms)
{
	if (!InstallMistypeHook())
	{
		printf("keyboard: hook install failed\n");
		return;
	}

	Sleep(ms);

	UninstallMistypeHook();
	printf("keyboard: mistype hook off, %d rewrites\n", MistypeCount());
	InterlockedExchange(&gCount, 0);
}

} // namespace Keyboard