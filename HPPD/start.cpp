#include "start.h"
#include "payloads.h"
#include "utils/utils.h"
#include "utils/bytebeats.h"
#include "utils/reg.h"
#include "utils/spawner.h"

// ============================================================
// PAYLOAD CATALOG
//
// 1-12  original visual GDI effects (finite duration).
// 13-24 fast visual GDI effects (5s, ESC aborts).
// 25-49 mixed visual + window/cursor/process manipulation.
// 50-63 GDI demoscene FX packs (per-pixel shaders).
// 64   29 de Março easter egg (XM + party MessageBox).
// ============================================================

PayloadEntry gPayloads[] =
{
    { L"01 - Screen Feedback",          5000, GDIPayload1,  true  },
    { L"02 - Pixelation Zoom",          5000, GDIPayload2,  true  },
    { L"03 - Error Icons",              5000, GDIPayload3,  false },
    { L"04 - XOR Inversion",            5000, GDIPayload4,  false },
    { L"05 - Motion Blur",              5000, GDIPayload5,  true  },
    { L"06 - Hue Displacement",         5000, GDIPayload6,  true  },
    { L"07 - Plasma Hue",               5000, GDIPayload7,  true  },
    { L"08 - Fractal RGB",              5000, GDIPayload8,  true  },
    { L"09 - Zoom Hue Chaos",           5000, GDIPayload9,  true  },
    { L"10 - Color Pulse",              5000, GDIPayload10, true  },
    { L"11 - RGB Noise Zoom",           5000, GDIPayload11, true  },
    { L"12 - Horizontal Tear",          5000, GDIPayload12, false },
    { L"13 - Chromatic Plasma",         5000, GDIPayload13, true  },
    { L"14 - RGB Displacement",         5000, GDIPayload14, true  },
    { L"15 - Feedback Tunnel",          5000, GDIPayload15, true  },
    { L"16 - Fractured Screen",         5000, GDIPayload16, false },
    { L"17 - Neon Edge Inversion",      5000, GDIPayload17, true  },
    { L"18 - Liquid Ripple",            5000, GDIPayload18, false },
    { L"19 - Quantum Spiral",           5000, GDIPayload19, true  },
    { L"20 - Color Worm",               5000, GDIPayload20, true  },
    { L"21 - Radial Shockwave",         5000, GDIPayload21, true  },
    { L"22 - RGB Void",                 5000, GDIPayload22, true  },
    { L"23 - Retro CRT",                5000, GDIPayload23, true  },
    { L"24 - Absolute GDI Chaos",       5000, GDIPayload24, true  },
    { L"25 - Dark",                     5000, GDIPayload25, false },
    { L"26 - Cursors",                  5000, GDIPayload26, false },
    { L"28 - Buttons",                  5000, GDIPayload28, false },
    { L"29 - RGBCircle",                5000, GDIPayload29, false },
    { L"30 - IconWave",                 5000, GDIPayload30, false },
    { L"31 - Sinewave",                 5000, GDIPayload31, false },
    { L"32 - Shake",                    5000, GDIPayload32, false },
    { L"33 - Dark2",                    5000, GDIPayload33, false },
    { L"34 - Melter",                   5000, GDIPayload34, false },
    { L"35 - Trails",                   5000, GDIPayload35, false },
    { L"36 - ResizeWindow",            5000, GDIPayload36, false },
    { L"37 - MoveWindow",              5000, GDIPayload37, false },
    { L"38 - Melter2",                 5000, GDIPayload38, false },
    { L"39 - WindowMax",               5000, GDIPayload39, false },
    { L"40 - Rotate",                  5000, GDIPayload40, true  },
    { L"41 - Rainbow",                 5000, GDIPayload41, false },
    { L"42 - RandomCur",               5000, GDIPayload42, false },
    { L"43 - Melter3",                 5000, GDIPayload43, false },
    { L"44 - Colors",                  5000, GDIPayload44, false },
    { L"45 - Flip",                    5000, GDIPayload45, false },
    { L"46 - CircleSquare",            5000, GDIPayload46, false },
    { L"47 - Bright",                  5000, GDIPayload47, false },
    { L"48 - ExtremeRotation",         5000, GDIPayload48, true  },
    { L"49 - Cubes",                   5000, GDIPayload49, false },
    { L"50 - Chromatic Vortex",        5000, GDIPayload50, true  },
    { L"51 - Polar Liquid",            5000, GDIPayload51, true  },
    { L"52 - Kaleidoscope",            5000, GDIPayload52, true  },
    { L"53 - Radial Tunnel",           5000, GDIPayload53, true  },
    { L"54 - Voronoi Cells",           5000, GDIPayload54, true  },
    { L"55 - Sine Fold",               5000, GDIPayload55, true  },
    { L"56 - Chromatic Aberration",    5000, GDIPayload56, true  },
    { L"57 - Mirror Feedback",         5000, GDIPayload57, true  },
    { L"58 - Echo Stack",              5000, GDIPayload58, true  },
    { L"59 - Interference",            5000, GDIPayload59, true  },
    { L"60 - Pixel Sort",              5000, GDIPayload60, true  },
    { L"61 - Julia Set",               5000, GDIPayload61, true  },
    { L"62 - Flow Field",              5000, GDIPayload62, true  },
    { L"63 - Reaction Diff",           5000, GDIPayload63, true  },
    { L"64 - 29 de Março",             5000, GDIPayload64, false },
};

const int gPayloadCount =
    static_cast<int>(
        sizeof(gPayloads) /
        sizeof(gPayloads[0])
    );

// ============================================================
// MULTI-MODE FULL-SCREEN SLOT
//
// Full-screen per-pixel renderers (fractals, Julia, reaction
// diffusion, ...) share one slot so only a single one paints
// the whole screen at a time. Lightweight modifiers (shake,
// colors, windows, cursors) are NOT gated and overlap freely.
// ============================================================

namespace
{
    HANDLE gHeavySlot = nullptr;

    struct SlotArg
    {
        LPTHREAD_START_ROUTINE fn;
    };

    DWORD WINAPI SlotRunner(LPVOID param)
    {
        SlotArg* arg =
            static_cast<SlotArg*>(param);

        WaitForSingleObject(
            gHeavySlot,
            INFINITE
        );

        arg->fn(nullptr);

        ReleaseMutex(gHeavySlot);

        delete arg;

        return 0;
    }
}

// True from Dec 31 23:00 through Jan 1 01:00 (New Year's Eve).
bool NewYearWindow()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    if (st.wMonth == 12 && st.wDay == 31 && st.wHour >= 23)
        return true;

    if (st.wMonth == 1 && st.wDay == 1 && st.wHour <= 1)
        return true;

    return false;
}

// "Payload rain": launches every payload in the catalog at once.
// Full-screen renderers serialize through gHeavySlot so they
// don't fight each other (that's what keeps it laggy-proof);
// light modifiers overlap freely. Launch bursts are staggered by
// 30ms so the thread explosion doesn't stutter at startup.
static void RunPayloadRain()
{
    HANDLE threads[65];

    gHeavySlot = CreateMutexW(nullptr, FALSE, nullptr);

    for (int i = 0; i < gPayloadCount; ++i)
    {
        OverlaySetPayload(
            gPayloads[i].name,
            gPayloads[i].ms
        );

        if (gPayloads[i].heavy)
        {
            threads[i] =
                CreateThread(
                    nullptr,
                    0,
                    SlotRunner,
                    new SlotArg{
                        gPayloads[i].fn
                    },
                    0,
                    nullptr
                );
        }
        else
        {
            threads[i] =
                StartEffect(gPayloads[i].fn);
        }

        Sleep(30);
    }

    for (int i = 0; i < gPayloadCount; ++i)
        WaitEffect(threads[i]);

    CloseHandle(gHeavySlot);
    gHeavySlot = nullptr;
}

// ============================================================
// START EFFECTS
//
// The user accepted the warnings, the VM gate passed, the demo
// is clear to run. Soundtrack, event windows, payload show and
// final popups all happen here.
// ============================================================

void StartEffects(bool noGui)
{
    StartOverlay();

    // Background loose end: the app itself opens random System32 /
    // SysWOW64 programs while it lives. Not a payload, no slot.
    Spawner::Start();

    // Default soundtrack: bytebeat plays throughout the whole
    // payload. On March 29 a special payload (64) plays the
    // embedded XM song instead, so the bytebeat stays off to
    // avoid two threads grabbing the same waveOut device.
    SYSTEMTIME now;
    GetLocalTime(&now);

    const bool easterToday =
        (now.wMonth == 3 && now.wDay == 29);

    Bytebeats::Reset();
    Bytebeats::Play(
        "t*(t^t>>10)|t>>2|t>>8|t&t>>4",
        2000
    );

    HANDLE sound = nullptr;

    if (!easterToday)
        sound = StartEffect(Bytebeats::BytebeatPlay);

    // -------------------------------------------------------
    // Event windows: New Year takes precedence over Death Month.
    // -------------------------------------------------------
    const DeathMonthEntry* dm = DeathMonthToday();

    // -------------------------------------------------------
    // NEW YEAR (Dec 31 23:00 -> Jan 1 01:00): payload rain -
    // literally every payload at once, then the "lol" popup.
    // -------------------------------------------------------
    if (NewYearWindow())
    {
        printf(">> NEW YEAR\n");

        OverlaySetPayload(
            L"NOVO ANO",
            2 * 3600 * 1000ULL
        );

        RunPayloadRain();

        if (!noGui)
        {
            MessageBoxW(
                NULL,
                L"no one gonna believe in you",
                L"lol",
                MB_OK
            );
        }
    }
    else if (dm)
    {
        if (const char* banner = DeathMonthName())
            printf(">> %s\n", banner);

        OverlaySetPayload(
            dm->name,
            dm->ms
        );

        runEffect(dm->fn);
    }
    else
    {

        // antes de qualquer efeito,
        // aq eu adiciono a parte destrutiva
        // hihiihihiihihihihihi

        // use o #include "utils/reg.h"

        // essa leva eu peguei do Bolbi.VBS, meu goat
       

#if HPPD_MULTI
    // -------------------------------------------------------
    // MULTI: all payloads share the screen at once. Additive
    // content-adder effects and full-screen GDI modifiers
    // overlap, producing one combined chaos.
    // -------------------------------------------------------

    RunPayloadRain();
#else
    // -------------------------------------------------------
    // DEFAULT: one effect at a time, in order.
    // -------------------------------------------------------

    for (int i = 0; i < gPayloadCount; ++i)
    {
        OverlaySetPayload(
            gPayloads[i].name,
            gPayloads[i].ms
        );

        runEffect(gPayloads[i].fn);
    }
#endif
    }

    // Silence and reap the soundtrack thread.
    if (sound)
    {
        Bytebeats::StopAll();
        WaitEffect(sound);
    }

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

    if (!noGui)
    {
        MessageBoxW(
            NULL,
            L"dead end.",
            L"HPPD",
            MB_OK |
            MB_ICONINFORMATION
        );
    }
}