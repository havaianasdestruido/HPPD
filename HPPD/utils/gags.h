#pragma once

#include <windows.h>

// ============================================================
// Named gag routines that orchestrate the other utils modules
// for a few seconds each. All of them clean up after themselves
// and target the current desktop only.
// ============================================================

namespace Gags {

// Tipsy mode: higher mistype rate, drifting mouse, stray beeps.
void Drunk(DWORD ms = 5000);

// Everything at once, briefly: inverted colors, blast of beeps,
// click storm, cycling cursors. Restores all of it.
void Panic(DWORD ms = 4000);

// Optical drive open/close dance.
void CdrRomPrank();

}