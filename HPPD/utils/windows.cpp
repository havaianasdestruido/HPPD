#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "windows.h"

namespace Windows2
{

namespace
{

std::vector<HWND> gDanceWindows;
std::vector<RECT> gDanceRects;

BOOL CALLBACK CollectProc(HWND hwnd, LPARAM lParam)
{
	auto* out = reinterpret_cast<std::vector<HWND>*>(lParam);
	if (IsWindowVisible(hwnd))
		out->push_back(hwnd);
	return TRUE;
}

} // namespace

void ListAll()
{
	std::vector<HWND> wins;
	EnumWindows(CollectProc, reinterpret_cast<LPARAM>(&wins));

	int n = 0;
	for (HWND hwnd : wins)
	{
		wchar_t title[256] = L"";
		GetWindowTextW(hwnd, title, 256);

		wchar_t cls[128] = L"";
		GetClassNameW(hwnd, cls, 128);

		RECT rc{};
		GetWindowRect(hwnd, &rc);

		printf("window: %-10p vis=%d %ls (%ls) %dx%d @ %d,%d\n",
			hwnd, IsWindowVisible(hwnd) ? 1 : 0, title, cls,
			rc.right - rc.left, rc.bottom - rc.top, rc.left, rc.top);
		++n;
	}

	printf("window: %d top-level total\n", n);
}

void Dance(DWORD ms)
{
	EnumWindows(CollectProc, reinterpret_cast<LPARAM>(&gDanceWindows));

	if (gDanceWindows.empty())
		return;

	gDanceRects.clear();
	for (HWND hwnd : gDanceWindows)
	{
		RECT rc{};
		GetWindowRect(hwnd, &rc);
		gDanceRects.push_back(rc);
	}

	const int vw = GetSystemMetrics(SM_CXSCREEN);
	const int vh = GetSystemMetrics(SM_CYSCREEN);

	srand(GetTickCount());

	for (DWORD t = 0; t < ms; t += 120)
	{
		for (size_t i = 0; i < gDanceWindows.size(); ++i)
		{
			const RECT& base = gDanceRects[i];
			const int w = base.right - base.left;
			const int h = base.bottom - base.top;

			int x = base.left + (rand() % 121) - 60;
			int y = base.top + (rand() % 121) - 60;

			x = x < 0 ? 0 : x;
			y = y < 0 ? 0 : y;
			if (x + w > vw) x = vw - w;
			if (y + h > vh) y = vh - h;

			SetWindowPos(gDanceWindows[i], nullptr, x, y, w, h,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		Sleep(120);
	}

	for (size_t i = 0; i < gDanceWindows.size(); ++i)
	{
		const RECT& rc = gDanceRects[i];
		SetWindowPos(gDanceWindows[i], nullptr, rc.left, rc.top,
			rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}

	gDanceRects.clear();
	gDanceWindows.clear();
}

void RetitleAll(DWORD ms, int maxTitles)
{
	static const LPCWSTR kTitlePool[] =
	{
		L"sigmaxxing",
		L"deu ruim",
		L"now loading...",
		L"¯\\_(ツ)_/¯",
		L"help",
		L"tatakae",
		L"this is fine",
		L"prompt ai",
	};

	std::vector<HWND> wins;
	EnumWindows(CollectProc, reinterpret_cast<LPARAM>(&wins));

	std::vector<std::wstring> bases;
	std::vector<HWND> targets;

	for (HWND hwnd : wins)
	{
		if (static_cast<int>(targets.size()) >= maxTitles)
			break;

		wchar_t title[256] = L"";
		GetWindowTextW(hwnd, title, 256);

		if (title[0] && wcscmp(title, L"HPPD") != 0)
		{
			targets.push_back(hwnd);
			bases.emplace_back(title);
		}
	}

	srand(GetTickCount());

	for (DWORD t = 0; t < ms; t += 250)
	{
		for (HWND hwnd : targets)
		{
			const LPCWSTR gag =
				kTitlePool[rand() % (sizeof(kTitlePool) / sizeof(kTitlePool[0]))];
			SetWindowTextW(hwnd, gag);
		}
		Sleep(250);
	}

	for (size_t i = 0; i < targets.size(); ++i)
		SetWindowTextW(targets[i], bases[i].c_str());
}

bool HideTaskbar()
{
	HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
	if (!tray)
		return false;
	ShowWindow(tray, SW_HIDE);
	return true;
}

bool ShowTaskbar()
{
	HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);
	if (!tray)
		return false;
	ShowWindow(tray, SW_SHOW);
	return true;
}

} // namespace Windows2