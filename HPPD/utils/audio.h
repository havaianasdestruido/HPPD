#pragma once

#include <windows.h>

// ============================================================
// Audio controls: waveOut master volume + beep melodies.
//
// Uses the classic winmm wave mapper, so it behaves the same on
// XP as on W10. Volume covers the master output scalar; it does
// not touch per-app mixers.
// ============================================================

namespace Audio {

// Number of standalone wave output devices.
int DeviceCount();

// Master volume as left/right 16-bit halves (WAVE_MAPPER).
bool GetVolume(DWORD& left, DWORD& right);
bool SetVolume(DWORD left, DWORD right);

void Mute();      // both channels to 0
void MaxVolume(); // both channels to full

// Short annoying beep gag. Blocking for ~1.5s.
void BlastBeeps();

// Ramp the volume down to mute over `ms` (and leave it there).
void FadeOut(DWORD ms);

}