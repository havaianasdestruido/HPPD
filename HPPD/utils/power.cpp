#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <powrprof.h>
#include <cstdio>

#include "power.h"

#pragma comment(lib, "powrprof.lib")

namespace Power
{

bool Standby()
{
	// SetSuspendState(FALSE, FALSE, FALSE) on XP/W10 is the same
	// as SetSystemPowerState(POWER_FORCE); no hibernation file.
	return SetSuspendState(FALSE, FALSE, FALSE) ? true : false;
}

bool Hibernate()
{
	return SetSuspendState(TRUE, FALSE, FALSE) ? true : false;
}

void MonitorOff()
{
	PostMessageW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);
}

void MonitorOn()
{
	PostMessageW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1);
}

DWORD PrintBattery()
{
	SYSTEM_POWER_STATUS sps{};
	if (!GetSystemPowerStatus(&sps))
	{
		printf("power: battery query failed\n");
		return 255;
	}

	const char* ac = "on-battery";
	if (sps.ACLineStatus == 1)
		ac = "on-AC";
	else if (sps.ACLineStatus == 255)
		ac = "unknown";

	printf("power: %s, charge %u%%, status 0x%02X\n",
		ac, sps.BatteryLifePercent, sps.BatteryFlag);

	return sps.BatteryLifePercent;
}

bool IsOnBattery()
{
	SYSTEM_POWER_STATUS sps{};
	return GetSystemPowerStatus(&sps)
		&& sps.ACLineStatus == 0
		&& sps.BatteryLifePercent != 255;
}

void PreventSleep()
{
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
}

void AllowSleep()
{
	SetThreadExecutionState(ES_CONTINUOUS);
}

} // namespace Power