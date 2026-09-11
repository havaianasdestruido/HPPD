#pragma once

#include <windows.h>

// ============================================================
// Keyboard mistype prank.
//
// Installs a WH_KEYBOARD_LL hook; on every keystroke there is a
// 1-in-N chance the key is rewritten to the PHYSICAL NEIGHBOR of
// the key that was pressed (Q <-> W, I <-> O, "," <-> ".", and
// so on). Because the replacement is positional, it is layout
// aware: on AZERTY the character that lands matches the key that
// physically sits next door, same as on QWERTY.
//
// Only plain typing keys are candidates — letters, digits and
// punctuation rows. F1-F12, arrows, modifiers, numpad, etc. are
// never touched.
//
// The installing process must pump messages for the hook to
// fire; the demo always does.
// ============================================================

namespace Keyboard {

// Install the mistype hook. Safe to call more than once.
bool InstallMistypeHook();

// Kill the hook (no-op if not installed).
void UninstallMistypeHook();

bool IsMistypeHookInstalled();

// One-in-N probability per keystroke. Default 100 (1%).
// Clamped to [1, 10000]. N=0 means "never".
void SetMistypeChance(int oneInN);

// How many keys were rewritten since install.
int MistypeCount();

// Convenience payload: run the prank for `ms` then stop.
void MistypeUntil(DWORD ms);

}