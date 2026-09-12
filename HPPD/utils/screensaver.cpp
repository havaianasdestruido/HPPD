#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#include "screensaver.h"

namespace ScrSave
{

namespace
{

bool gArmed = false;
UINT gOldTimeout = 60;
BOOL gOldActive = FALSE;

} // namespace

bool ForceNow(DWORD holdMs)
{
	// Capture the current conditions.
	SystemParametersInfoW(SPI_GETSCREENSAVETIMEOUT, 0, &gOldTimeout, 0);
	SystemParametersInfoW(SPI_GETSCREENSAVEACTIVE, 0, &gOldActive, 0);

	if (!gOldActive)
		return false;

	// Drop the timeout to 1s so the OS fires almost immediately.
	if (!SystemParametersInfoW(SPI_SETSCREENSAVETIMEOUT, 1, nullptr, 0))
		return false;

	gArmed = true;

	// Nudge the desktop into starting the saver right now.
	SendMessageTimeoutW(
		HWND_BROADCAST,
		WM_SYSCOMMAND,
		SC_SCREENSAVE,
		0,
		SMTO_ABORTIFHUNG,
		1000,
		nullptr);

	if (holdMs)
	{
		Sleep(holdMs);
		Restore();
	}

	return true;
}

void Restore()
{
	if (!gArmed)
		return;

	SystemParametersInfoW(SPI_SETSCREENSAVETIMEOUT, gOldTimeout, nullptr, 0);
	SystemParametersInfoW(SPI_SETSCREENSAVEACTIVE, gOldActive, nullptr, 0);
	gArmed = false;
}

bool Armed()
{
	return gArmed;
}

} // namespace ScrSave