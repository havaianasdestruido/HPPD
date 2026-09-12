#pragma once

#include <windows.h>

// ============================================================
// System cursor shenanigans. All swaps are done with standard
// cursors (no .cur files needed) and are always restorable via
// SystemParametersInfoW(SPI_SETCURSORS).
// ============================================================

namespace Cursor {

// Scramble the six standard cursors among themselves.
bool SwapStandard();

// Cycle a random cursor every 300ms for `ms`, then restore.
void Chaos(DWORD ms);

// Restore the default cursor scheme.
void Restore();

// Is a Cursor swap currently armed?
bool Armed();

}