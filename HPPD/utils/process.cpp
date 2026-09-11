#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>
#include <cstdlib>

#include "process.h"

// SetPriorityClass values live in winbase; keep the header lean.
#ifndef PROCESS_SET_INFORMATION
#define PROCESS_SET_INFORMATION 0x0200
#endif

namespace Process
{

namespace
{

DWORD FindPid(int index, const wchar_t* name = nullptr)
{
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE)
		return 0;

	PROCESSENTRY32W pe{};
	pe.dwSize = sizeof(pe);

	int n = -1;
	DWORD pid = 0;

	if (Process32FirstW(snap, &pe))
	{
		do
		{
			++n;
			if (name && _wcsicmp(pe.szExeFile, name) == 0)
			{
				pid = pe.th32ProcessID;
				break;
			}
			if (!name && n == index)
			{
				pid = pe.th32ProcessID;
				break;
			}
		} while (Process32NextW(snap, &pe));
	}

	CloseHandle(snap);
	return pid;
}

} // namespace

void List()
{
	HANDLE snap = CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD, 0);
	if (snap == INVALID_HANDLE_VALUE)
		return;

	PROCESSENTRY32W pe{};
	pe.dwSize = sizeof(pe);

	int count = 0;
	if (Process32FirstW(snap, &pe))
	{
		do
		{
			printf("process: %-6lu %-4lu %ls\n",
				pe.th32ProcessID, pe.cntThreads, pe.szExeFile);
			++count;
		} while (Process32NextW(snap, &pe));
	}

	CloseHandle(snap);
	printf("process: %d total\n", count);
}

int FindIndex(const wchar_t* name)
{
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE)
		return -1;

	PROCESSENTRY32W pe{};
	pe.dwSize = sizeof(pe);

	int n = -1;
	if (Process32FirstW(snap, &pe))
	{
		do
		{
			++n;
			if (_wcsicmp(pe.szExeFile, name) == 0)
			{
				CloseHandle(snap);
				return n;
			}
		} while (Process32NextW(snap, &pe));
	}

	CloseHandle(snap);
	return -1;
}

static bool SetSuspendState(DWORD pid, bool suspend)
{
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (snap == INVALID_HANDLE_VALUE)
		return false;

	THREADENTRY32 te{};
	te.dwSize = sizeof(te);

	bool any = false;
	if (Thread32First(snap, &te))
	{
		do
		{
			if (te.th32OwnerProcessID != pid)
				continue;

			HANDLE h = OpenThread(
				THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
			if (!h)
				continue;

			if (suspend)
				SuspendThread(h);
			else
				ResumeThread(h);

			CloseHandle(h);
			any = true;
		} while (Thread32Next(snap, &te));
	}

	CloseHandle(snap);
	return any;
}

bool SuspendPid(DWORD pid)
{
	return SetSuspendState(pid, true);
}

bool ResumePid(DWORD pid)
{
	return SetSuspendState(pid, false);
}

bool Suspend(int index)
{
	const DWORD pid = FindPid(index);
	return pid ? SuspendPid(pid) : false;
}

bool Resume(int index)
{
	const DWORD pid = FindPid(index);
	return pid ? ResumePid(pid) : false;
}

bool SuspendByName(const wchar_t* name)
{
	const DWORD pid = FindPid(0, name);
	return pid ? SuspendPid(pid) : false;
}

bool ResumeByName(const wchar_t* name)
{
	const DWORD pid = FindPid(0, name);
	return pid ? ResumePid(pid) : false;
}

int KillByName(const wchar_t* name)
{
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snap == INVALID_HANDLE_VALUE)
		return 0;

	PROCESSENTRY32W pe{};
	pe.dwSize = sizeof(pe);

	int killed = 0;
	if (Process32FirstW(snap, &pe))
	{
		do
		{
			if (_wcsicmp(pe.szExeFile, name) != 0)
				continue;

			HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
			if (h)
			{
				if (TerminateProcess(h, 0))
					++killed;
				CloseHandle(h);
			}
		} while (Process32NextW(snap, &pe));
	}

	CloseHandle(snap);
	return killed;
}

void PriorityChaos(const wchar_t* name, DWORD ms)
{
	static const DWORD kPriorities[] =
	{
		IDLE_PRIORITY_CLASS,
		BELOW_NORMAL_PRIORITY_CLASS,
		ABOVE_NORMAL_PRIORITY_CLASS,
		HIGH_PRIORITY_CLASS,
		REALTIME_PRIORITY_CLASS
	};

	const DWORD pid = FindPid(0, name);
	if (!pid)
		return;

	HANDLE h = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);
	if (!h)
		return;

	srand(GetTickCount());

	for (DWORD t = 0; t < ms; t += 120)
	{
		SetPriorityClass(h, kPriorities[rand() % 5]);
		Sleep(120);
	}

	SetPriorityClass(h, NORMAL_PRIORITY_CLASS);
	CloseHandle(h);
}

} // namespace Process