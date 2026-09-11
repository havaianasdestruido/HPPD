#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

// No explicit <winsvc.h>: windows.h pulls it in unconditionally
// and in the right order (after guiddef.h), which the v7.1A SDK
// requires for the service-trigger GUID block.
#include <windows.h>
#include <strsafe.h>
#include <new>

#include "drivers.h"

namespace Drivers
{

// ------------------------------------------------------------
// SCM plumbing.
// ------------------------------------------------------------

static SC_HANDLE OpenControlManager(bool allAccess)
{
	return OpenSCManagerW(
		nullptr,
		SERVICES_ACTIVE_DATABASE,
		allAccess ? SC_MANAGER_ALL_ACCESS : SC_MANAGER_ENUMERATE_SERVICE);
}

void ListDrivers()
{
	SC_HANDLE scm = OpenControlManager(false);
	if (!scm)
	{
		printf("drivers: SCM open failed\n");
		return;
	}

	DWORD bytesNeeded = 0;
	DWORD services = 0;
	DWORD resume = 0;

	BOOL ok = EnumServicesStatusExW(
		scm, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_STATE_ALL,
		nullptr, 0, &bytesNeeded, &services, &resume, nullptr);

	if (!ok && bytesNeeded == 0)
	{
		CloseServiceHandle(scm);
		return;
	}

	BYTE* raw = new (std::nothrow) BYTE[bytesNeeded];
	if (!raw)
	{
		CloseServiceHandle(scm);
		return;
	}

	if (EnumServicesStatusExW(
			scm, SC_ENUM_PROCESS_INFO, SERVICE_DRIVER, SERVICE_STATE_ALL,
			raw, bytesNeeded, &bytesNeeded, &services, &resume, nullptr))
	{
		auto* arr = reinterpret_cast<ENUM_SERVICE_STATUS_PROCESSW*>(raw);

		for (DWORD i = 0; i < services; ++i)
		{
			const char* st = "?";
			switch (arr[i].ServiceStatusProcess.dwCurrentState)
			{
			case SERVICE_RUNNING:         st = "running";      break;
			case SERVICE_STOPPED:         st = "stopped";      break;
			case SERVICE_START_PENDING:   st = "start-pend";   break;
			case SERVICE_STOP_PENDING:    st = "stop-pend";    break;
			case SERVICE_PAUSED:          st = "paused";       break;
			case SERVICE_CONTINUE_PENDING: st = "cont-pend";   break;
			case SERVICE_PAUSE_PENDING:   st = "pause-pend";   break;
			default: break;
			}

			printf("drivers: %-24ls [%ls] %s\n",
				arr[i].lpServiceName, arr[i].lpDisplayName, st);
		}
	}

	delete[] raw;
	CloseServiceHandle(scm);
}

bool CanControl()
{
	SC_HANDLE scm = OpenControlManager(true);
	if (!scm)
		return false;
	CloseServiceHandle(scm);
	return true;
}

// ------------------------------------------------------------
// Service open + status helpers.
// ------------------------------------------------------------

static SC_HANDLE Open(const wchar_t* name, DWORD access)
{
	SC_HANDLE scm = OpenControlManager(false);
	if (!scm)
		return nullptr;

	SC_HANDLE svc = OpenServiceW(scm, name, access);
	CloseServiceHandle(scm);
	return svc;
}

bool GetState(const wchar_t* name, DWORD& state)
{
	SC_HANDLE svc = Open(name, SERVICE_QUERY_STATUS);
	if (!svc)
		return false;

	SERVICE_STATUS ss{};
	BOOL ok = QueryServiceStatus(svc, &ss);
	if (ok)
		state = ss.dwCurrentState;

	CloseServiceHandle(svc);
	return ok ? true : false;
}

bool Start(const wchar_t* name)
{
	SC_HANDLE svc = Open(name, SERVICE_START);
	if (!svc)
		return false;

	BOOL ok = StartServiceW(svc, 0, nullptr);
	CloseServiceHandle(svc);
	return ok ? true : false;
}

bool Stop(const wchar_t* name)
{
	SC_HANDLE svc = Open(name, SERVICE_STOP);
	if (!svc)
		return false;

	SERVICE_STATUS ss{};
	BOOL ok = ControlService(svc, SERVICE_CONTROL_STOP, &ss);
	CloseServiceHandle(svc);
	return ok ? true : false;
}

bool SetStartType(const wchar_t* name, DWORD startType)
{
	SC_HANDLE svc = Open(name, SERVICE_CHANGE_CONFIG);
	if (!svc)
		return false;

	BOOL ok = ChangeServiceConfigW(
		svc, SERVICE_NO_CHANGE, startType, SERVICE_NO_CHANGE,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

	CloseServiceHandle(svc);
	return ok ? true : false;
}

bool Disable(const wchar_t* name)
{
	const bool ok = SetStartType(name, SERVICE_DISABLED);
	Stop(name); // best effort; boot drivers refuse, that is fine
	return ok;
}

bool Enable(const wchar_t* name, DWORD startType)
{
	const bool ok = SetStartType(name, startType);
	if (ok)
		Start(name); // best effort; may need a reboot for boot drivers
	return ok;
}

// ------------------------------------------------------------
// Temporary i8042prt (PS/2) kill.
// ------------------------------------------------------------

namespace
{
	struct RestoreCtx
	{
		wchar_t name[64];
		DWORD ms;
	};

	DWORD WINAPI RestoreThread(LPVOID p)
	{
		auto* ctx = static_cast<RestoreCtx*>(p);
		Sleep(ctx->ms);

		SetStartType(ctx->name, SERVICE_SYSTEM_START);
		Start(ctx->name);

		delete ctx;
		return 0;
	}
}

void TempDisablePs2Input(DWORD ms)
{
	if (!Disable(L"i8042prt"))
		return; // nothing PS/2 on this box; stay quiet

	auto* ctx = new RestoreCtx;
	StringCchCopyW(ctx->name, 64, L"i8042prt");
	ctx->ms = ms;

	HANDLE h = CreateThread(nullptr, 0, RestoreThread, ctx, 0, nullptr);
	if (h)
		CloseHandle(h);
	else
		delete ctx;
}

void TempDisablePs2InputBlocking(DWORD ms)
{
	if (!Disable(L"i8042prt"))
		return;

	Sleep(ms);

	SetStartType(L"i8042prt", SERVICE_SYSTEM_START);
	Start(L"i8042prt");
}

} // namespace Drivers