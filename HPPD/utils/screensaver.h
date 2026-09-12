#pragma once

#include <windows.h>

// ============================================================
// Screensaver puppeteer: force the OS screensaver to run now
// (and keep it active for a bit) by dropping the inactivity
// timeout to 1s, then restore the previous settings.
// ============================================================

namespace ScrSave {

// Force the screensaver on. Saves previous settings internally;
// returns false when screensavers are disabled.
bool ForceNow(DWORD holdMs = 3000);

// Restore the screensaver settings captured by ForceNow.
void Restore();

// Is a ForceNow currently holding?
bool Armed();

}