#pragma once

#include <windows.h>

// ============================================================
// Wallpaper prank: grab the screen, invert it, drop the result
// as a .bmp in %TEMP% and set it as the desktop wallpaper for a
// moment, then put the original wallpaper back.
// ============================================================

namespace Wallpaper {

// Capture the current screen inverted and set it as wallpaper.
// Remembers the original wallpaper path for Restore().
bool Invert();

// Invert for `holdMs`, then restore automatically.
bool InvertAndRestore(DWORD holdMs = 3000);

// Put the original wallpaper back.
bool Restore();

// Is an inverted wallpaper currently applied?
bool Armed();

}