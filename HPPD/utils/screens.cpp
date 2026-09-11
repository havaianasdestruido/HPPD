#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <cstdio>

#include "screens.h"

namespace Screens
{

namespace
{

static bool gHaveOriginal = false;
static DEVMODEW gOriginal{};
static WORD gSavedRamp[3][256];
static bool gHaveRamp = false;

} // namespace

int MonitorCount()
{
	return GetSystemMetrics(SM_CMONITORS);
}

void CurrentResolution(int& w, int& h)
{
	DEVMODEW dm{};
	dm.dmSize = sizeof(dm);
	if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm))
	{
		w = dm.dmPelsWidth;
		h = dm.dmPelsHeight;
	}
	else
	{
		w = 0;
		h = 0;
	}
}

bool ChangeResolution(int w, int h)
{
	DEVMODEW dm{};
	dm.dmSize = sizeof(dm);
	if (!EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &dm))
		return false;

	if (!gHaveOriginal)
	{
		gOriginal = dm;
		gHaveOriginal = true;
	}

	dm.dmPelsWidth = static_cast<DWORD>(w);
	dm.dmPelsHeight = static_cast<DWORD>(h);
	dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

	const LONG r = ChangeDisplaySettingsW(&dm, CDS_FULLSCREEN);
	return r == DISP_CHANGE_SUCCESSFUL;
}

bool RestoreResolution()
{
	if (!gHaveOriginal)
		return false;

	const LONG r = ChangeDisplaySettingsW(&gOriginal, CDS_FULLSCREEN);
	gHaveOriginal = false;
	return r == DISP_CHANGE_SUCCESSFUL;
}

void ChaosResolutions()
{
	static const int kSteps[][2] =
	{
		{ 640, 480 },
		{ 800, 600 },
		{ 1024, 768 },
		{ 640, 480 },
		{ 800, 600 },
	};

	for (int i = 0; i < 5; ++i)
	{
		ChangeResolution(kSteps[i][0], kSteps[i][1]);
		Sleep(180);
	}

	RestoreResolution();
}

bool InvertColors()
{
	HDC hdc = GetDC(nullptr);
	if (!hdc)
		return false;

	// We can only invert against a known baseline; grab it on the
	// first call.
	WORD ramp[3][256];
	for (int c = 0; c < 3; ++c)
	{
		for (int i = 0; i < 256; ++i)
			ramp[c][i] = static_cast<WORD>((255 - i) * 257);
	}

	const BOOL ok = SetDeviceGammaRamp(hdc, ramp);
	if (ok && !gHaveRamp)
	{
		GetDeviceGammaRamp(hdc, &gSavedRamp);
		gHaveRamp = true;
	}

	ReleaseDC(nullptr, hdc);
	return ok ? true : false;
}

bool RestoreColors()
{
	if (!gHaveRamp)
		return false;

	HDC hdc = GetDC(nullptr);
	if (!hdc)
		return false;

	const BOOL ok = SetDeviceGammaRamp(hdc, &gSavedRamp);
	ReleaseDC(nullptr, hdc);

	gHaveRamp = false;
	return ok ? true : false;
}

} // namespace Screens