#pragma once

#include <cstdint>
#include <windows.h>

// ============================================================
// Global runtime SEED system.
//
// gSeed is picked once when the program starts and printed to
// the console. Every payload derives a stable per-payload
// variant from it (phase / dir / amp / rng) that nudges its
// formula so each run looks different.
//
// All math here is cheap integer/bit work; it must never run
// per-pixel. Variants are computed lazily and cached on first
// use of each payload.
// ============================================================

extern uint32_t gSeed;

struct SeedVariant
{
    int phase;       // 0..2047  extra phase for periodic/wave formulas
    int dir;         // -1 or +1  sign flip
    int amp;         // 1..8      amplitude / loop / zoom tweak
    uint32_t rng;    // nonzero   derived state for FXRand / srand
};

// Returns the cached variant for a payload id.
const SeedVariant& SeedGet(int payloadId);

// Seed value to pass to srand() so rand()-based payloads vary per run.
static inline unsigned SeedRand(int payloadId)
{
    return static_cast<unsigned>(
        SeedGet(payloadId).rng
    );
}