#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <cstdio>
#include <intrin.h>

#include "utils.h"
#include "payloads.h"
#include "seed.h"

// ============================================================
// Safe visual-only GDI effects demo
//
// No:
// - disk writes
// - MBR modification
// - privilege escalation
// - BSOD / NtRaiseHardError
// - audio playback
//
// ESC aborts Payloads 13-24.
// Payloads 1-12 retain their original finite durations.
//
// A global SEED is picked once at startup and printed to the
// console. Each payload derives its own variant from it, so
// every run of the same effects looks different.
// ============================================================

// ============================================================
// MAIN
// ============================================================

int main()
{
    gSeed =
        static_cast<uint32_t>(
            GetTickCount64()
        ) ^ static_cast<uint32_t>(__rdtsc());

    printf("Global SEED: %u\n", gSeed);
    fflush(stdout);

    int result =
        MessageBoxW(
            NULL,
            L"This is a visual-effects demo.\r\n\r\n"
            L"It does not modify your disk, crash Windows, "
            L"change privileges, or play audio.\r\n\r\n"
            L"Press ESC during Payloads 13-24 "
            L"to skip the current effect.\r\n\r\n"
            L"Press Ctrl+F2 to toggle the payload "
            L"name / time overlay.\r\n\r\n"
            L"Continue?",
            L"Bromine Visual Demo",
            MB_YESNO |
            MB_ICONINFORMATION
        );

    if (result != IDYES)
        return 0;

    StartOverlay();

    // --------------------------------------------------------
    // New high-performance Payloads
    // --------------------------------------------------------

    OverlaySetPayload(L"13 - Chromatic Plasma", 5000);
    runEffect(GDIPayload13);
    OverlaySetPayload(L"14 - RGB Displacement", 5000);
    runEffect(GDIPayload14);
    OverlaySetPayload(L"15 - Feedback Tunnel", 5000);
    runEffect(GDIPayload15);
    OverlaySetPayload(L"16 - Fractured Screen", 5000);
    runEffect(GDIPayload16);
    OverlaySetPayload(L"17 - Neon Edge Inversion", 5000);
    runEffect(GDIPayload17);
    OverlaySetPayload(L"18 - Liquid Ripple", 5000);
    runEffect(GDIPayload18);
    OverlaySetPayload(L"19 - Quantum Spiral", 5000);
    runEffect(GDIPayload19);
    OverlaySetPayload(L"20 - Color Worm", 5000);
    runEffect(GDIPayload20);
    OverlaySetPayload(L"21 - Radial Shockwave", 5000);
    runEffect(GDIPayload21);
    OverlaySetPayload(L"22 - RGB Void", 5000);
    runEffect(GDIPayload22);
    OverlaySetPayload(L"23 - Retro CRT", 5000);
    runEffect(GDIPayload23);
    OverlaySetPayload(L"24 - Absolute GDI Chaos", 5000);
    runEffect(GDIPayload24);

    // --------------------------------------------------------
    // Final cleanup
    // --------------------------------------------------------

    RedrawWindow(
        NULL,
        NULL,
        NULL,
        RDW_INVALIDATE |
        RDW_ERASE |
        RDW_UPDATENOW |
        RDW_ALLCHILDREN
    );

    MessageBoxW(
        NULL,
        L"Visual demo finished.",
        L"Bromine Visual Demo",
        MB_OK |
        MB_ICONINFORMATION
    );

    return 0;
}
