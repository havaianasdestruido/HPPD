#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <cstdio>
#include <intrin.h>

#include "utils.h"
#include "payloads.h"
#include "seed.h"

// ============================================================
// HPPD - effects demo
//
// No:
// - disk writes
// - MBR modification
// - privilege escalation
// - BSOD / NtRaiseHardError
// - audio playback
//
// Payloads 27, 28, 35, 36, 37, 39, 42 do manipulate windows
// the user owns (resize / move / disable Close / spawn a few
// harmless Windows utilities). They are still safe in the
// sense of "won't kill the machine", but they will fight the
// user while they run.
//
// ESC aborts Payloads 13-49.
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
            L"Press ESC to skip the current effect.\r\n\r\n"
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
    // Payload 13-24 table (13-24 run; 1-12 are kept commented
    // out for compatibility with the default build).
    // --------------------------------------------------------

    struct
    {
        const wchar_t* name;
        DWORD ms;
    } payloads[] =
    {
        { L"13 - Chromatic Plasma",        5000 },
        { L"14 - RGB Displacement",        5000 },
        { L"15 - Feedback Tunnel",         5000 },
        { L"16 - Fractured Screen",        5000 },
        { L"17 - Neon Edge Inversion",     5000 },
        { L"18 - Liquid Ripple",           5000 },
        { L"19 - Quantum Spiral",          5000 },
        { L"20 - Color Worm",              5000 },
        { L"21 - Radial Shockwave",        5000 },
        { L"22 - RGB Void",                5000 },
        { L"23 - Retro CRT",               5000 },
        { L"24 - Absolute GDI Chaos",      5000 },
        { L"25 - Dark",                    5000 },
        { L"26 - Cursors",                 5000 },
        { L"27 - Programs",                5000 },
        { L"28 - Buttons",                 5000 },
        { L"29 - RGBCircle",               5000 },
        { L"30 - IconWave",                5000 },
        { L"31 - Sinewave",                5000 },
        { L"32 - Shake",                   5000 },
        { L"33 - Dark2",                   5000 },
        { L"34 - Melter",                  5000 },
        { L"35 - Trails",                  5000 },
        { L"36 - ResizeWindow",            5000 },
        { L"37 - MoveWindow",              5000 },
        { L"38 - Melter2",                 5000 },
        { L"39 - WindowMax",               5000 },
        { L"40 - Rotate",                  5000 },
        { L"41 - Rainbow",                 5000 },
        { L"42 - RandomCur",               5000 },
        { L"43 - Melter3",                 5000 },
        { L"44 - Colors",                  5000 },
        { L"45 - Flip",                    5000 },
        { L"46 - CircleSquare",            5000 },
        { L"47 - Bright",                  5000 },
        { L"48 - ExtremeRotation",         5000 },
        { L"49 - Cubes",                   5000 },
    };

    LPTHREAD_START_ROUTINE effects[] =
    {
        GDIPayload13,
        GDIPayload14,
        GDIPayload15,
        GDIPayload16,
        GDIPayload17,
        GDIPayload18,
        GDIPayload19,
        GDIPayload20,
        GDIPayload21,
        GDIPayload22,
        GDIPayload23,
        GDIPayload24,
        GDIPayload25,
        GDIPayload26,
        GDIPayload27,
        GDIPayload28,
        GDIPayload29,
        GDIPayload30,
        GDIPayload31,
        GDIPayload32,
        GDIPayload33,
        GDIPayload34,
        GDIPayload35,
        GDIPayload36,
        GDIPayload37,
        GDIPayload38,
        GDIPayload39,
        GDIPayload40,
        GDIPayload41,
        GDIPayload42,
        GDIPayload43,
        GDIPayload44,
        GDIPayload45,
        GDIPayload46,
        GDIPayload47,
        GDIPayload48,
        GDIPayload49,
    };

    const int count =
        static_cast<int>(
            sizeof(effects) /
            sizeof(effects[0])
        );

#if HPPD_MULTI
    // -------------------------------------------------------
    // MULTI: all payloads share the screen at once. Additive
    // content-adder effects and full-screen GDI modifiers
    // overlap, producing one combined chaos.
    // -------------------------------------------------------

    HANDLE threads[32];

    for (int i = 0; i < count; ++i)
    {
        OverlaySetPayload(
            payloads[i].name,
            payloads[i].ms
        );

        threads[i] =
            StartEffect(effects[i]);
    }

    for (int i = 0; i < count; ++i)
        WaitEffect(threads[i]);
#else
    // -------------------------------------------------------
    // DEFAULT: one effect at a time, in order.
    // -------------------------------------------------------

    for (int i = 0; i < count; ++i)
    {
        OverlaySetPayload(
            payloads[i].name,
            payloads[i].ms
        );

        runEffect(effects[i]);
    }
#endif

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
