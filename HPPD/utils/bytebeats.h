#pragma once

#include <windows.h>

// ============================================================
// BYTEBEAT SOUND
//
// One self-contained 8-bit PCM waveOut loop playing
//   t*(t^t>>10)|t>>2|t>>8|t&t>>4
// at 2000 Hz. Runs as a thread so the visual GDI payloads keep
// playing at the same time as the music.
//
// Usage:
//   Bytebeats::Reset();
//   HANDLE h = StartEffect(Bytebeats::BytebeatPlay);
//   ...
//   Bytebeats::StopAll();
//   WaitEffect(h);
//
// ESC and the HPPD STOP latch also silence it.
// ============================================================

namespace Bytebeats
{
    DWORD WINAPI BytebeatPlay(LPVOID lpvd);

    // Clears the stop latch. Call before starting the tune.
    void Reset();

    // Sets the shared stop latch. The tune resets its audio
    // device and exits shortly after.
    void StopAll();
}