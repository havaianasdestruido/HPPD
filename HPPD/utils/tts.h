#pragma once

#include <windows.h>

// ============================================================
// Speech module: SAPI 5.1 through ISpVoice.
//
// Windows ships SAPI 5.1 since XP, so the demo can talk on any
// target. COM is initialized here; callers never touch COM.
// ============================================================

namespace Tts {

// Synchronous: speaks `text` and waits for completion.
bool Speak(const wchar_t* text);

// Fire-and-forget (returns after queuing).
bool SpeakAsync(const wchar_t* text);

// Block until queued speech is done.
void Wait();

// Cancel anything still being spoken.
void Stop();

// Current rate in -10..10 (0 = default).
int Rate();
bool SetRate(int rate);

}