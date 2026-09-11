#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <strsafe.h>
#include <shlobj.h>
#include <string>

#include "browser.h"

namespace Browser
{

// ------------------------------------------------------------
// Small registry writer.
// ------------------------------------------------------------

static bool SetRegString(HKEY root, const wchar_t* path, const wchar_t* name,
	const wchar_t* value)
{
	HKEY k = nullptr;
	LONG r = RegCreateKeyExW(
		root, path, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &k, nullptr);
	if (r != ERROR_SUCCESS)
		return false;

	const DWORD bytes =
		static_cast<DWORD>((lstrlenW(value) + 1) * sizeof(wchar_t));
	r = RegSetValueExW(
		k, name, 0, REG_SZ, reinterpret_cast<const BYTE*>(value), bytes);

	RegCloseKey(k);
	return r == ERROR_SUCCESS;
}

static bool SetRegDword(HKEY root, const wchar_t* path,
	const wchar_t* name, DWORD value)
{
	HKEY k = nullptr;
	LONG r = RegCreateKeyExW(
		root, path, 0, nullptr, 0, KEY_SET_VALUE, nullptr, &k, nullptr);
	if (r != ERROR_SUCCESS)
		return false;

	r = RegSetValueExW(
		k, name, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value),
		sizeof(DWORD));

	RegCloseKey(k);
	return r == ERROR_SUCCESS;
}

// ------------------------------------------------------------
// Internet Explorer.
// ------------------------------------------------------------

static const wchar_t kIeMainKey[] =
	L"Software\\Microsoft\\Internet Explorer\\Main";

static const wchar_t kIeTypedUrlsKey[] =
	L"Software\\Microsoft\\Internet Explorer\\TypedURLs";

bool SetIEHomepage(const wchar_t* url)
{
	return SetRegString(HKEY_CURRENT_USER, kIeMainKey, L"Start Page", url);
}

bool AddIETypedUrl(const wchar_t* url)
{
	HKEY k = nullptr;
	LONG r = RegCreateKeyExW(
		HKEY_CURRENT_USER, kIeTypedUrlsKey, 0, nullptr, 0,
		KEY_QUERY_VALUE | KEY_SET_VALUE, nullptr, &k, nullptr);
	if (r != ERROR_SUCCESS)
		return false;

	// Find the highest existing urlN, then write the next slot
	// (IE renumbers these itself when it is run).
	constexpr int kMaxSlots = 25;
	int next = 1;

	for (int i = 1; i <= kMaxSlots; ++i)
	{
		wchar_t nm[16];
		StringCchPrintfW(nm, 16, L"url%d", i);

		DWORD type = 0, unused = 0;
		if (RegQueryValueExW(k, nm, nullptr, &type, nullptr, &unused)
			== ERROR_SUCCESS)
			next = i + 1;
	}

	if (next > kMaxSlots)
	{
		RegCloseKey(k);
		return false;
	}

	wchar_t nm[16];
	StringCchPrintfW(nm, 16, L"url%d", next);

	const DWORD bytes =
		static_cast<DWORD>((lstrlenW(url) + 1) * sizeof(wchar_t));
	r = RegSetValueExW(
		k, nm, 0, REG_SZ, reinterpret_cast<const BYTE*>(url), bytes);

	// Timestamp entry, same shape IE writes.
	FILETIME ft{};
	GetSystemTimeAsFileTime(&ft);
	ULARGE_INTEGER ui;
	ui.LowPart = ft.dwLowDateTime;
	ui.HighPart = ft.dwHighDateTime;

	StringCchPrintfW(nm, 16, L"url%d", next);
	RegSetValueExW(
		k, nm, 0, REG_QWORD, reinterpret_cast<const BYTE*>(&ui.QuadPart),
		sizeof(ui.QuadPart));

	RegCloseKey(k);
	return r == ERROR_SUCCESS;
}

int AddGagTypedUrls(int count)
{
	static const wchar_t* const kGagSites[] =
	{
		L"https://en.wikipedia.org/wiki/Lorem_ipsum",
		L"https://www.youtube.com/watch?v=dQw4w9WgXcQ",
		L"https://www.iana.org/domains/reserved",
		L"https://example.com/",
		L"https://www.wikipedia.org",
		L"https://developer.mozilla.org/en-US/docs/Web",
	};

	const int sites =
		static_cast<int>(sizeof(kGagSites) / sizeof(kGagSites[0]));

	int added = 0;
	for (int i = 0; i < count; ++i)
	{
		if (AddIETypedUrl(kGagSites[i % sites]))
			++added;
	}

	return added;
}

// ------------------------------------------------------------
// Chromium family (Chrome / Edge). Policy keys override the
// user preference, which is exactly the point.
// ------------------------------------------------------------

bool SetChromiumHomepage(const wchar_t* url)
{
	bool ok = false;

	ok |= SetRegString(
		HKEY_CURRENT_USER,
		L"Software\\Policies\\Google\\Chrome",
		L"HomepageLocation", url);
	SetRegDword(
		HKEY_CURRENT_USER,
		L"Software\\Policies\\Google\\Chrome",
		L"HomepageIsNewTabPage", 0);

	ok |= SetRegString(
		HKEY_CURRENT_USER,
		L"Software\\Policies\\Microsoft\\Edge",
		L"HomepageLocation", url);
	SetRegDword(
		HKEY_CURRENT_USER,
		L"Software\\Policies\\Microsoft\\Edge",
		L"HomepageIsNewTabPage", 0);

	return ok;
}

// ------------------------------------------------------------
// Firefox: patch the profile's prefs.js.
// ------------------------------------------------------------

static std::string Utf8FromWide(const wchar_t* w)
{
	const int needed = WideCharToMultiByte(
		CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
	if (needed <= 1)
		return std::string();

	std::string out(static_cast<size_t>(needed - 1), '\0');
	WideCharToMultiByte(
		CP_UTF8, 0, w, -1, &out[0], needed - 1, nullptr, nullptr);
	return out;
}

bool SetFirefoxHomepage(const wchar_t* url)
{
	wchar_t appData[MAX_PATH];
	if (!SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appData))
		return false;

	wchar_t profilesDir[MAX_PATH];
	StringCchPrintfW(profilesDir, MAX_PATH,
		L"%s\\Mozilla\\Firefox\\Profiles", appData);

	wchar_t pattern[MAX_PATH];
	StringCchPrintfW(pattern, MAX_PATH, L"%s\\*", profilesDir);

	WIN32_FIND_DATAW fd{};
	HANDLE hFind = FindFirstFileW(pattern, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	const wchar_t kNeedle[] = L"prefs.js";
	wchar_t prefsPath[MAX_PATH];
	bool found = false;

	do
	{
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		StringCchPrintfW(prefsPath, MAX_PATH,
			L"%s\\%s\\%s", profilesDir, fd.cFileName, kNeedle);

		if (GetFileAttributesW(prefsPath) != INVALID_FILE_ATTRIBUTES)
		{
			found = true;
			break;
		}
	} while (FindNextFileW(hFind, &fd));

	FindClose(hFind);
	if (!found)
		return false;

	// Read the whole file, drop the old homepage line, append ours.
	HANDLE f = CreateFileW(
		prefsPath, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr, OPEN_EXISTING, 0, nullptr);
	if (f == INVALID_HANDLE_VALUE)
		return false;

	DWORD size = GetFileSize(f, nullptr);
	if (size == INVALID_FILE_SIZE)
		size = 0;

	std::string content(static_cast<size_t>(size), '\0');
	if (size)
	{
		DWORD read = 0;
		ReadFile(f, &content[0], size, &read, nullptr);
		content.resize(read);
	}

	// Remove any existing user_pref(...browser.startup.homepage...)
	size_t pos = 0;
	while ((pos = content.find("browser.startup.homepage", pos))
		!= std::string::npos)
	{
		size_t lineStart = content.rfind('\n', pos);
		lineStart = (lineStart == std::string::npos)
			? 0 : lineStart + 1;

		size_t lineEnd = content.find('\n', pos);
		lineEnd = (lineEnd == std::string::npos)
			? content.size() : lineEnd + 1;

		content.erase(lineStart, lineEnd - lineStart);
		pos = lineStart;
	}

	if (!content.empty() && content.back() != '\n')
		content.push_back('\n');

	const std::string utf8Url = Utf8FromWide(url);
	content += "user_pref(\"browser.startup.homepage\", \"";
	content += utf8Url;
	content += "\");\n";

	DWORD written = 0;
	const BOOL ok = WriteFile(
		f, content.data(), static_cast<DWORD>(content.size()), &written, nullptr)
		&& SetEndOfFile(f);

	CloseHandle(f);
	return ok ? true : false;
}

// ------------------------------------------------------------
// Kitchen sink.
// ------------------------------------------------------------

bool SetAllHomepages(const wchar_t* url)
{
	bool ok = SetIEHomepage(url);
	ok |= SetChromiumHomepage(url);
	ok |= SetFirefoxHomepage(url);
	return ok;
}

const wchar_t* DefaultGagUrl()
{
	return L"https://www.youtube.com/watch?v=dQw4w9WgXcQ";
}

} // namespace Browser