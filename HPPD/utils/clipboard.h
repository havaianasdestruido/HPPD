#pragma once

#include <windows.h>

// ============================================================
// Clipboard prank utilities.
//
// Straight WM_CLIPBOARD access — works identically on XP and
// W10. Nothing here depends on modern shell APIs.
// ============================================================

namespace Clipboard {

// Copies the current CF_UNICODETEXT into `out` (cch includes the
// null terminator). false when the clipboard has no text or is
// busy.
bool Capture(wchar_t* out, int cch);

// Returns the length in wchar_ts of the current text (0 if none).
int Length();

// Replaces the clipboard text. true on success.
bool SetText(const wchar_t* text);

// Gag: capture, swap every adjacent pair of characters, put it
// back. Returns how many pairs were swapped.
int SwapAdjacentLetters();

// Prints the clipboard text to the console.
void Show();

}