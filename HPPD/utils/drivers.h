#pragma once

#include <windows.h>

// ============================================================
// Kernel driver (kernel-mode service) control.
//
// Everything goes through the Service Control Manager, so the
// write operations need an elevated process (the demo
// self-elevates; without rights they just return false).
//
// The classic prank target is i8042prt: the controller driver
// for PS/2 keyboards AND mice. Disabling it removes PS/2 input
// until it is re-enabled. On machines without PS/2 devices the
// calls fail harmlessly.
// ============================================================

namespace Drivers {

// Prints every kernel driver and its state to the console.
void ListDrivers();

// true when the process can change driver/service configuration.
bool CanControl();

// Reads a driver's current state into `state`
// (SERVICE_STOPPED / SERVICE_RUNNING / ...). false on error.
bool GetState(const wchar_t* name, DWORD& state);

// Start / stop a driver service.
bool Start(const wchar_t* name);
bool Stop(const wchar_t* name);

// Change the driver's start type. SERVICE_DISABLED turns it off,
// SERVICE_BOOT_START / SERVICE_SYSTEM_START / SERVICE_DEMAND_START
// restore it.
bool SetStartType(const wchar_t* name, DWORD startType);

// Disable a driver so it stops loading, and stop it if possible.
bool Disable(const wchar_t* name);

// Re-enable a driver (defaults back to SYSTEM_START).
bool Enable(const wchar_t* name, DWORD startType = SERVICE_SYSTEM_START);

// Prank: disable i8042prt for `ms`, then a worker thread restores
// SYSTEM_START and tries to start it again.
void TempDisablePs2Input(DWORD ms);

// Blocking variant: the caller sleeps instead of a worker thread.
void TempDisablePs2InputBlocking(DWORD ms);

}