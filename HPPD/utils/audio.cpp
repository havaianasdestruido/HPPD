#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <mmsystem.h>
#include <cstdio>

#include "audio.h"

#pragma comment(lib, "winmm.lib")

namespace Audio
{

int DeviceCount()
{
	return waveOutGetNumDevs();
}

const HWAVEOUT kMapper = reinterpret_cast<HWAVEOUT>(WAVE_MAPPER);

bool GetVolume(DWORD& left, DWORD& right)
{
	DWORD dw = 0;
	if (waveOutGetVolume(kMapper, &dw) != MMSYSERR_NOERROR)
		return false;

	left = LOWORD(dw);
	right = HIWORD(dw);
	return true;
}

bool SetVolume(DWORD left, DWORD right)
{
	const DWORD dw = MAKELONG(left, right);
	return waveOutSetVolume(kMapper, dw) == MMSYSERR_NOERROR;
}

void Mute()
{
	SetVolume(0, 0);
}

void MaxVolume()
{
	SetVolume(0xFFFF, 0xFFFF);
}

void BlastBeeps()
{
	// Classic annoyance: 1kHz + 2kHz ping-pong.
	for (int i = 0; i < 4; ++i)
	{
		Beep(1000, 120);
		Beep(2000, 120);
	}
}

void FadeOut(DWORD ms)
{
	const DWORD step = ms / 16;
	if (step == 0)
	{
		Mute();
		return;
	}

	for (int i = 15; i >= 0; --i)
	{
		const DWORD level = static_cast<DWORD>(i * i * 255); // i^2*255 -> up to 57375? scaled below
		const DWORD scaled = (level * 0xFFFF) / (15 * 15 * 255);
		SetVolume(scaled, scaled);
		Sleep(step);
	}
}

} // namespace Audio