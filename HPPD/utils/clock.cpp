#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <cstdio>
#include <cstdlib>

#include "clock.h"

namespace Clock
{

namespace
{

static SYSTEMTIME gSaved{};
static bool gHaveSaved = false;

// Enables a privilege in the current process token.
bool EnablePrivilege(const wchar_t* name)
{
	HANDLE token = nullptr;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
		return false;

	LUID luid{};
	if (!LookupPrivilegeValueW(nullptr, name, &luid))
	{
		CloseHandle(token);
		return false;
	}

	TOKEN_PRIVILEGES tp{};
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	const BOOL ok = AdjustTokenPrivileges(
		token, FALSE, &tp, 0, nullptr, nullptr);

	CloseHandle(token);
	return ok && GetLastError() == ERROR_SUCCESS;
}

} // namespace

void Print()
{
	SYSTEMTIME st{};
	GetLocalTime(&st);

	printf("clock: %02u/%02u/%04u %02u:%02u:%02u\n",
		st.wDay, st.wMonth, st.wYear,
		st.wHour, st.wMinute, st.wSecond);
}

bool CanSet()
{
	return EnablePrivilege(SE_SYSTEMTIME_NAME);
}

bool Set(const SYSTEMTIME& st)
{
	if (!EnablePrivilege(SE_SYSTEMTIME_NAME))
		return false;

	if (SetLocalTime(&st))
	{
		if (!gHaveSaved)
		{
			GetLocalTime(&gSaved);
			gHaveSaved = true;
		}
		return true;
	}

	return false;
}

bool SetRandomOffset()
{
	SYSTEMTIME st{};
	GetLocalTime(&st);

	srand(GetTickCount());

	st.wHour = (st.wHour + (rand() % 9) + 2) % 24;
	st.wMinute = rand() % 60;
	st.wDay = (st.wDay % 28) + 1; // stay valid in any month

	return Set(st);
}

bool RollbackToMillennium()
{
	SYSTEMTIME st{};
	GetLocalTime(&st);

	st.wYear = 2000;
	st.wMonth = 1;
	st.wDay = 1;
	st.wDayOfWeek = 0;
	st.wHour = 0;
	st.wMinute = 0;
	st.wSecond = 0;
	st.wMilliseconds = 0;

	return Set(st);
}

bool Restore()
{
	if (!gHaveSaved)
		return false;

	const bool ok = Set(gSaved);
	gHaveSaved = false;
	return ok;
}

} // namespace Clock