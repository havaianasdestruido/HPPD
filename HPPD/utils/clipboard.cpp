#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <strsafe.h>
#include <cstdio>

#include "clipboard.h"

namespace Clipboard
{

static wchar_t* LockText()
{
	if (!OpenClipboard(nullptr))
		return nullptr;

	HANDLE h = GetClipboardData(CF_UNICODETEXT);
	if (!h)
	{
		CloseClipboard();
		return nullptr;
	}

	wchar_t* p = static_cast<wchar_t*>(GlobalLock(h));
	if (!p)
		CloseClipboard();

	return p; // caller must GlobalUnlock + CloseClipboard
}

static void Unlock(wchar_t* p)
{
	GlobalUnlock(p);
	CloseClipboard();
}

int Length()
{
	wchar_t* p = LockText();
	if (!p)
		return 0;

	const int n = lstrlenW(p);
	Unlock(p);
	return n;
}

bool Capture(wchar_t* out, int cch)
{
	if (!out || cch <= 0)
		return false;

	wchar_t* p = LockText();
	if (!p)
		return false;

	StringCchCopyW(out, cch, p);
	Unlock(p);
	return true;
}

bool SetText(const wchar_t* text)
{
	if (!OpenClipboard(nullptr))
		return false;

	if (!EmptyClipboard())
	{
		CloseClipboard();
		return false;
	}

	const size_t cch = wcslen(text) + 1;
	HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, cch * sizeof(wchar_t));
	if (!h)
	{
		CloseClipboard();
		return false;
	}

	wchar_t* p = static_cast<wchar_t*>(GlobalLock(h));
	CopyMemory(p, text, cch * sizeof(wchar_t));
	GlobalUnlock(h);

	HANDLE ok = SetClipboardData(CF_UNICODETEXT, h);
	CloseClipboard();

	if (!ok)
		GlobalFree(h); // clipboard did not take ownership

	return ok != nullptr;
}

int SwapAdjacentLetters()
{
	wchar_t* p = LockText();
	if (!p)
		return 0;

	const int n = lstrlenW(p);
	int swapped = 0;

	for (int i = 0; i + 1 < n; i += 2)
	{
		if (p[i] != p[i + 1])
		{
			const wchar_t t = p[i];
			p[i] = p[i + 1];
			p[i + 1] = t;
			++swapped;
		}
	}

	Unlock(p); // CF_UNICODETEXT is global memory; edit-in-place persists
	return swapped;
}

void Show()
{
	wchar_t* p = LockText();
	if (!p)
		return;

	printf("clipboard: \"%ls\"\n", p);
	Unlock(p);
}

} // namespace Clipboard