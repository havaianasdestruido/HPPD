#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#include "dates.h"

#include "../utils/unicurse.h" // needed for cursed payloads

// Mass eject system

namespace MassEject // ik making a full-ass namespace is a overkill
{					// but who cares lol

static int EnumDisks(wchar_t roots[][4], int maxDrives)
{
	// Fills up to `maxDrives` entries with drive-root strings
	// ("C:\", "D:\", ...). Returns how many were filled.
	DWORD mask = GetLogicalDrives();
	int n = 0;

	for (int i = 0; i < 26 && n < maxDrives; ++i)
	{
		if (mask & (1u << i))
		{
			wsprintfW(roots[n], L"%c:\\", L'A' + i);
			++n;
		}
	}

	return n;
}
static void ForceEject(int whatever) //put a string or anything as a param
{
	// TODO: real force ejecting logic
    // (without any messageboxes envolved)
    MessageBoxW(
        NULL,
        L"Goodbye to " + whatever,
        L"lol",
        MB_OK |
        MB_ICONWARNING
    );
}
static void ForceFormat(int whatever) //put a string or anything as a param
{
    // TODO: real formatting logic
    // (without any messageboxes envolved)
    MessageBoxW(
        NULL,
        L"Goodbye to " + whatever,
        L"xd",
        MB_OK |
        MB_ICONWARNING
    );
}
}