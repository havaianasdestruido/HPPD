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
