#pragma once

#include <windows.h>

// ============================================================
// Synthetic input (SendInput). Complements the Keyboard module,
// which only rewrites real keystrokes — here we CREATE events.
//
// XP and W10 identical; SendInput needs UIPI elevation parity
// (an admin sender cannot inject into elevated apps), so the
// non-elevated demo affects normal apps only.
// ============================================================

namespace Input {

// Move the cursor and press/release the left button once.
void Click(int x, int y);

// Random clicks for `ms` anywhere on the virtual screen.
void ClickStorm(DWORD ms);

// Send a string via KEYEVENTF_UNICODE keystrokes (focus where it
// matters, i.e. the foreground window).
void TypeText(const wchar_t* text);

// Small random cursor jumps around the current position.
void NudgeMouse(int range);

// Toggle CapsLock state. Returns the new state (0/1).
bool ToggleCapsLock();

}