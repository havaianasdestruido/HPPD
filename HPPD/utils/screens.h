#pragma once

#include <windows.h>

// ============================================================
// Screen / display pranks: resolution and gamma.
//
// ChangeDisplaySettingsW and SetDeviceGammaRamp work on XP and
// W7; on W8+ the GDI gamma ramp is ignored (DWM owns the composit-
// ion), but resolution changes still apply. Every call remembers
// the previous state for a matching Restore.
// ============================================================

namespace Screens {

// Number of monitors in the virtual screen.
int MonitorCount();

// Current primary resolution, or 0x0 on failure.
void CurrentResolution(int& w, int& h);

// Shrink-grow the desktop. true when the OS accepted it.
bool ChangeResolution(int w, int h);
bool RestoreResolution();

// Gag: cycle through a couple of sad resolutions fast, then
// restore whatever was active when the gag started.
void ChaosResolutions();

// Invert the RGB gamma curve on the primary display.
bool InvertColors();
bool RestoreColors();

}