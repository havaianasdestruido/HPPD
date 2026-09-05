#include "seed.h"

uint32_t gSeed = 0;

// Cheap integer avalanche mix (xorshift-style). No FPU use,
// no memory, fast enough to run anywhere but only used once
// per payload on first access.
static uint32_t Mix32(uint32_t v)
{
    v ^= v << 13;
    v ^= v >> 17;
    v ^= v << 5;

    v *= 0x85EBCA6Bu;

    v ^= v >> 16;

    return v;
}

const SeedVariant& SeedGet(int payloadId)
{
    static SeedVariant variants[25];
    static bool initialized = false;

    if (!initialized)
    {
        for (int p = 0; p <= 24; ++p)
        {
            uint32_t h =
                Mix32(
                    gSeed ^
                    static_cast<uint32_t>(p) *
                    0x9E3779B9u
                );

            variants[p].phase =
                static_cast<int>(h & 2047);

            variants[p].dir =
                (h & (1u << 20))
                    ? 1
                    : -1;

            variants[p].amp =
                1 +
                static_cast<int>(
                    (h >> 11) & 7
                );

            variants[p].rng =
                h | 1u;
        }

        initialized = true;
    }

    return variants[payloadId];
}