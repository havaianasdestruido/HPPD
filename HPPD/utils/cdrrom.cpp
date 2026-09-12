#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <mmsystem.h>
#include <cstdio>

#include "cdrrom.h"

#pragma comment(lib, "winmm.lib")

namespace CdrRom
{

static int FirstCdrm()
{
	for (int i = 0; i < 26; ++i)
	{
		char root[4] = { static_cast<char>('A' + i), ':', '\\', 0 };
		if (GetDriveTypeA(root) == DRIVE_CDROM)
			return i;
	}
	return -1;
}

int Count()
{
	int n = 0;
	for (int i = 0; i < 26; ++i)
	{
		char root[4] = { static_cast<char>('A' + i), ':', '\\', 0 };
		if (GetDriveTypeA(root) == DRIVE_CDROM)
			++n;
	}
	return n;
}

static bool Door(bool open)
{
	const int drive = FirstCdrm();
	if (drive < 0)
		return false;

	wchar_t cmd[64];

	// Close any lingering alias first (MCIERR_OPEN if left open).
	wsprintfW(cmd, L"close hppdt%d", drive);
	mciSendStringW(cmd, nullptr, 0, nullptr);

	wsprintfW(cmd, L"open %c: type cdaudio alias hppdt%d",
		L'A' + drive, drive);
	mciSendStringW(cmd, nullptr, 0, nullptr);

	wsprintfW(cmd, L"set hppdt%d door %s", drive,
		open ? L"open" : L"closed");

	// mciSendStringW returns 0 on success.
	return mciSendStringW(cmd, nullptr, 0, nullptr) == 0;
}

bool Eject()
{
	return Door(true);
}

bool Close()
{
	return Door(false);
}

void EjectAll()
{
	for (int i = 0; i < 26; ++i)
	{
		char root[4] = { static_cast<char>('A' + i), ':', '\\', 0 };
		if (GetDriveTypeA(root) != DRIVE_CDROM)
			continue;

		wchar_t cmd[64];
		wsprintfW(cmd, L"open %c: type cdaudio alias hppdej%d",
			L'A' + i, i);
		mciSendStringW(cmd, nullptr, 0, nullptr);

		wsprintfW(cmd, L"set hppdej%d door open", i);
		mciSendStringW(cmd, nullptr, 0, nullptr);
	}
}

void CloseAll()
{
	for (int i = 0; i < 26; ++i)
	{
		wchar_t cmd[64];
		wsprintfW(cmd, L"set hppdej%d door closed", i);
		mciSendStringW(cmd, nullptr, 0, nullptr);
	}
}

void Prank(DWORD waitMs)
{
	EjectAll();
	Sleep(waitMs);
	CloseAll();
}

} // namespace CdrRom