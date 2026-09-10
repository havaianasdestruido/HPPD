#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

// ============================================================
// START
//
// Everything that happens once the user accepted the warning
// popup AND the VM gate passed — i.e. "let's run the effects".
// main.cpp handles bootstrapping (elevation, seed, easter eggs,
// VM check, warning popup) and then hands control to
// StartEffects(). The payload catalog lives here too so both the
// selector popup (main.cpp) and the demo loop can use it.
// ============================================================

struct PayloadEntry
{
    const wchar_t* name;
    DWORD ms;
    LPTHREAD_START_ROUTINE fn;
    bool heavy; // full-screen per-pixel renderer: runs one at a time
};

extern PayloadEntry gPayloads[];
extern const int gPayloadCount;

// True from Dec 31 23:00 through Jan 1 01:00 (New Year's Eve).
bool NewYearWindow();

// Runs the whole effects show: thumbnail bytebeat soundtrack,
// event windows (New Year / Death Month), then the regular demo.
// noGui repeats the flag from main() to decide final popups.
void StartEffects(bool noGui);