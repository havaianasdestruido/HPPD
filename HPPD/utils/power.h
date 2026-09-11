#pragma once

#include <windows.h>

// ============================================================
// Power control: standby / hibernate / monitor + battery recon.
//
// Preferences may block standby/hibernate on W10; on those the
// calls return false instead of panicking. MonitorOff() uses the
// classic SC_MONITORPOWER broadcast, which modern Windows only
// honors for elevated/trusted callers.
// ============================================================

namespace Power {

// Suspend to RAM / hibernate (best effort, both eras).
bool Standby();
bool Hibernate();

// Screen goes black until input arrives.
void MonitorOff();
void MonitorOn();

// Battery/AC recon. Prints a summary and returns the charge %.
DWORD PrintBattery();

// Convenience bools.
bool IsOnBattery();

// Hold the system awake (SetThreadExecutionState).
void PreventSleep();
void AllowSleep();

}