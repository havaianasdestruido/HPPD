#pragma once

#include <windows.h>

// ============================================================
// Top-level window chaos: listing, dancing, retitling and the
// taskbar gag. Every dance/retitle is reversible and restores
// positions/titles from values captured before the gag.
// ============================================================

namespace Windows2 {

// Print handle, visible state and title of every top-level window.
void ListAll();

// Drift every visible top-level window around for `ms`, then put
// it back where it started.
void Dance(DWORD ms);

// Rename up to `maxTitles` visible windows with gag titles,
// cycling during `ms`, then restore the originals.
void RetitleAll(DWORD ms, int maxTitles = 12);

// Show/hide the taskbar.
bool HideTaskbar();
bool ShowTaskbar();

}