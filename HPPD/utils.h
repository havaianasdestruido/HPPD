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
