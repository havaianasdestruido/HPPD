#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <cstdlib>

#include "input.h"

namespace Input
{

static void SendSingle(INPUT& in)
{
	SendInput(1, &in, sizeof(INPUT));
}

static void MouseDown(int x, int y, DWORD flags)
{
	SetCursorPos(x, y);

	INPUT in{};
	in.type = INPUT_MOUSE;
	in.mi.dwFlags = flags;
	SendSingle(in);
}

void Click(int x, int y)
{
	MouseDown(x, y, MOUSEEVENTF_LEFTDOWN);
	MouseDown(x, y, MOUSEEVENTF_LEFTUP);
}

void ClickStorm(DWORD ms)
{
	const int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	const int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	const int ox = GetSystemMetrics(SM_XVIRTUALSCREEN);
	const int oy = GetSystemMetrics(SM_YVIRTUALSCREEN);

	srand(GetTickCount());

	const DWORD step = 25;
	for (DWORD t = 0; t < ms; t += step)
	{
		const int x = ox + (w ? rand() % w : 0);
		const int y = oy + (h ? rand() % h : 0);

		MouseDown(x, y, MOUSEEVENTF_LEFTDOWN);
		MouseDown(x, y, MOUSEEVENTF_LEFTUP);

		Sleep(step);
	}
}

void TypeText(const wchar_t* text)
{
	for (const wchar_t* p = text; *p; ++p)
	{
		INPUT down{};
		down.type = INPUT_KEYBOARD;
		down.ki.wVk = 0;
		down.ki.wScan = static_cast<WORD>(*p);
		down.ki.dwFlags = KEYEVENTF_UNICODE;
		SendSingle(down);

		INPUT up{};
		up.type = INPUT_KEYBOARD;
		up.ki.wVk = 0;
		up.ki.wScan = static_cast<WORD>(*p);
		up.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
		SendSingle(up);

		Sleep(5);
	}
}

void NudgeMouse(int range)
{
	if (range < 1)
		range = 1;

	srand(GetTickCount());

	POINT p{};
	GetCursorPos(&p);

	p.x += (rand() % (2 * range + 1)) - range;
	p.y += (rand() % (2 * range + 1)) - range;

	SetCursorPos(p.x, p.y);
}

bool ToggleCapsLock()
{
	INPUT in{};
	in.type = INPUT_KEYBOARD;
	in.ki.wVk = VK_CAPITAL;

	SendSingle(in);
	SendSingle(in);

	return (GetKeyState(VK_CAPITAL) & 1) != 0;
}

} // namespace Input