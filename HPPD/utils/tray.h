#pragma once

#include <windows.h>

// ============================================================
// Tray joker: fake system-tray icon + balloon tip.
//
// XP and W10 behave the same for the classic NOTIFYICONDATA v1
// members we touch; the GUID/Realtime members are Vista+ and
// deliberately omitted.
// ============================================================

namespace Tray {

// Show a balloon tip for `timeoutMs`, then remove the icon.
// Blocking; false when the tray is unavailable.
bool Balloon(const wchar_t* title, const wchar_t* text, DWORD timeoutMs = 5000);

// Fire a balloon and return immediately (icon stays up until
// Dismiss() is called).
bool BalloonFire(const wchar_t* title, const wchar_t* text);

// Remove the icon created by BalloonFire.
void Dismiss();

}