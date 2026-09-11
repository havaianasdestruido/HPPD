#pragma once

#include <windows.h>

// ============================================================
// System-clock pranks.
//
// Setting the time requires the SE_SYSTEMTIME privilege; setting
// a time zone needs SE_TIME_ZONE_NAME. The demo self-elevates, so
// the token is usually admin — the privilege still has to be
// enabled per-adjust for the call to succeed. On non-admin the
// writers simply return false.
// ============================================================

namespace Clock {

// Prints the current local date/time to the console.
void Print();

// true when SE_SYSTEMTIME can be enabled in this process token.
bool CanSet();

// Apply `st` (local time) and keep the previous setting for
// Restore(). false when the privilege is missing.
bool Set(const SYSTEMTIME& st);

// Gag: add a random offset of a few hours/minutes.
bool SetRandomOffset();

// Gag: back to midnight, January 1st 2000.
bool RollbackToMillennium();

// Restore whatever Set() stored last (no-op if nothing stored).
bool Restore();

}