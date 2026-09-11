#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <objbase.h>
#include <cstdio>

#include "shell.h"

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

namespace ShellUtils
{

namespace
{

static wchar_t gFolderBuffer[MAX_PATH];

// CSIDL-based older folder lookup (XP-compatible).
const wchar_t* FolderByCsidl(int csidl)
{
	const HRESULT hr = SHGetFolderPathW(nullptr, csidl, nullptr, 0,
		gFolderBuffer);
	return SUCCEEDED(hr) ? gFolderBuffer : nullptr;
}

// Recycle-bin APIs were dropped from the 7.1A SDK headers; pull
// them from shell32 at runtime instead (present since Win98).
typedef DWORD(WINAPI* QueryRecycleBinFn)(PCWSTR, void*);
typedef DWORD(WINAPI* EmptyRecycleBinFn)(HWND, PCWSTR, DWORD);

struct SHQUERYRBINFO_ { DWORD cbSize; ULONGLONG i64Size; ULONGLONG i64NumItems; };

} // namespace

ULONGLONG RecycleBinBytes()
{
	HMODULE shell = GetModuleHandleW(L"shell32.dll");
	if (!shell)
		return 0;

	auto fn = reinterpret_cast<QueryRecycleBinFn>(
		GetProcAddress(shell, "SHQueryRecycleBinW"));
	if (!fn)
		return 0;

	SHQUERYRBINFO_ info{};
	info.cbSize = sizeof(info);

	if (fn(nullptr, &info) != 0)
		return 0;

	return info.i64Size;
}

bool EmptyRecycleBin()
{
	HMODULE shell = GetModuleHandleW(L"shell32.dll");
	if (!shell)
		return false;

	auto fn = reinterpret_cast<EmptyRecycleBinFn>(
		GetProcAddress(shell, "SHEmptyRecycleBinW"));
	if (!fn)
		return false;

	return fn(nullptr, nullptr, 9 /* NORMSGS|NOSOUND */) == 0;
}

bool DeleteToBin(const wchar_t* path)
{
	// SHFileOperationW is still declared by the SDK.
	wchar_t from[MAX_PATH + 2] = L"";
	wcsncpy_s(from, path, _TRUNCATE);
	from[wcslen(from) + 1] = L'\0'; // double-null terminated

	SHFILEOPSTRUCTW op{};
	op.wFunc = FO_DELETE;
	op.pFrom = from;
	op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

	return SHFileOperationW(&op) == 0;
}

bool CreateShortcut(const wchar_t* linkPath,
	const wchar_t* target,
	const wchar_t* workDir,
	const wchar_t* description)
{
	IShellLinkW* link = nullptr;
	const HRESULT hrCreate = CoCreateInstance(CLSID_ShellLink, nullptr,
		CLSCTX_INPROC_SERVER, IID_IShellLinkW,
		reinterpret_cast<void**>(&link));
	if (FAILED(hrCreate))
		return false;

	link->SetPath(target);
	if (workDir)
		link->SetWorkingDirectory(workDir);
	if (description)
		link->SetDescription(description);

	IPersistFile* file = nullptr;
	const HRESULT hrQI = link->QueryInterface(IID_IPersistFile,
		reinterpret_cast<void**>(&file));

	BOOL ok = FALSE;
	if (SUCCEEDED(hrQI))
	{
		ok = SUCCEEDED(file->Save(linkPath, TRUE));
		file->Release();
	}

	link->Release();
	return ok != FALSE;
}

bool CreateUrlShortcut(const wchar_t* linkPath, const wchar_t* url)
{
	FILE* f = _wfopen(linkPath, L"wb");
	if (!f)
		return false;

	fwprintf(f, L"[InternetShortcut]\r\nURL=%s\r\n", url);
	fclose(f);
	return true;
}

const wchar_t* DesktopPath()
{
	return FolderByCsidl(CSIDL_DESKTOPDIRECTORY);
}

const wchar_t* StartupPath()
{
	return FolderByCsidl(CSIDL_STARTUP);
}

} // namespace ShellUtils