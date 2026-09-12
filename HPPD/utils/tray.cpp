#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <shellapi.h>
#include <strsafe.h>

#include "tray.h"

#pragma comment(lib, "shell32.lib")

namespace Tray
{

namespace
{

const UINT kMsg = WM_USER + 33;
const UINT kIconId = 1;

HWND gWnd = nullptr;

LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

HWND EnsureWindow()
{
	if (gWnd)
		return gWnd;

	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = TrayWndProc;
	wc.lpszClassName = L"HPPDTrayNotifyHost";
	wc.hInstance = GetModuleHandleW(nullptr);

	if (!RegisterClassExW(&wc))
	{
		const DWORD err = GetLastError();
		if (err != ERROR_CLASS_ALREADY_EXISTS)
			return nullptr;
	}

	gWnd = CreateWindowExW(0, L"HPPDTrayNotifyHost", L"", 0, 0, 0, 0, 0,
		HWND_MESSAGE, nullptr, wc.hInstance, nullptr);

	return gWnd;
}

NOTIFYICONDATAW MakeNid()
{
	NOTIFYICONDATAW nid{};
	nid.cbSize = NOTIFYICONDATAW_V1_SIZE;
	nid.uID = kIconId;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.hWnd = EnsureWindow();
	nid.uCallbackMessage = kMsg;
	nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
	nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;

	return nid;
}

} // namespace

bool BalloonFire(const wchar_t* title, const wchar_t* text)
{
	NOTIFYICONDATAW nid = MakeNid();
	if (!nid.hWnd)
		return false;

	StringCchCopyW(nid.szInfo, ARRAYSIZE(nid.szInfo), text);
	StringCchCopyW(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), title);
	nid.uTimeout = 10000;

	return Shell_NotifyIconW(NIM_ADD, &nid) == TRUE;
}

void Dismiss()
{
	NOTIFYICONDATAW nid = MakeNid();
	if (nid.hWnd)
		Shell_NotifyIconW(NIM_DELETE, &nid);
}

bool Balloon(const wchar_t* title, const wchar_t* text, DWORD timeoutMs)
{
	if (!BalloonFire(title, text))
		return false;

	Sleep(timeoutMs);
	Dismiss();
	return true;
}

} // namespace Tray