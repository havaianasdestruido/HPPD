#pragma once
#include <stdbool.h>
#include <Windows.h>

// ============================================================
// Existing helper
// ============================================================

BOOL CALLBACK EnumProc(HWND hWnd, LPARAM);

// ============================================================
// EFFECT RUNNER
// ============================================================

// Launch a payload on its own thread, return its handle.
// Returns NULL on failure.
HANDLE StartEffect(
    LPTHREAD_START_ROUTINE effect
);

// Wait for a launched effect to finish, then tidy up the screen.
void WaitEffect(
    HANDLE thread
);

// One-at-a-time wrapper: StartEffect + WaitEffect.
void runEffect(
    LPTHREAD_START_ROUTINE effect
);

// ============================================================
// TEXT/TIMEBAR OVERLAY (Ctrl+F2)
// ============================================================

void StartOverlay(); // spawns the overlay watchdog thread

void OverlaySetPayload(
    const wchar_t* name,
    ULONGLONG durationMs
);
