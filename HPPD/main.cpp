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
            L"Continue?",
            L"Bromine Visual Demo",
            MB_YESNO |
            MB_ICONINFORMATION
        );

    if (result != IDYES)
        return 0;

    // --------------------------------------------------------
    // Original Payloads
    // --------------------------------------------------------

    /*runEffect(GDIPayload1);
    runEffect(GDIPayload2);
    runEffect(GDIPayload3);
    runEffect(GDIPayload4);
    runEffect(GDIPayload5);
    runEffect(GDIPayload6);
    runEffect(GDIPayload7);
    runEffect(GDIPayload8);
    runEffect(GDIPayload9);
    runEffect(GDIPayload10);
    runEffect(GDIPayload11);
    runEffect(GDIPayload12);*/

    // --------------------------------------------------------
    // New high-performance Payloads
    // --------------------------------------------------------

    runEffect(GDIPayload13);
    runEffect(GDIPayload14);
    runEffect(GDIPayload15);
    runEffect(GDIPayload16);
    runEffect(GDIPayload17);
    runEffect(GDIPayload18);
    runEffect(GDIPayload19);
    runEffect(GDIPayload20);
    runEffect(GDIPayload21);
    runEffect(GDIPayload22);
    runEffect(GDIPayload23);
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
