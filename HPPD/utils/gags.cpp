#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <cstdlib>

#include "gags.h"

#include "audio.h"
#include "cdrrom.h"
#include "cursor.h"
#include "input.h"
#include "keyboard.h"
#include "screens.h"

namespace Gags
{

void Drunk(DWORD ms)
{
	Keyboard::SetMistypeChance(20); // 1-in-20, sloppy
	Keyboard::InstallMistypeHook();

	srand(GetTickCount());

	const DWORD step = 250;
	for (DWORD t = 0; t < ms; t += step)
	{
		Input::NudgeMouse(30);

		if ((rand() % 3) == 0)
			Beep(200 + (rand() % 600), 40);

		Sleep(step);
	}

	Keyboard::UninstallMistypeHook();
}

void Panic(DWORD ms)
{
	Screens::InvertColors();
	Cursor::Chaos(1200);

	const DWORD half = ms / 2;

	// Beeps + click storm concurrently-ish: blast first, then
	// flood the screen while the colors are still inverted.
	Audio::BlastBeeps();
	Input::ClickStorm(half);

	Screens::RestoreColors();
	Cursor::Restore();
}

void CdrRomPrank()
{
	CdrRom::EjectAll();
	Sleep(2500);
	CdrRom::CloseAll();
}

} // namespace Gags