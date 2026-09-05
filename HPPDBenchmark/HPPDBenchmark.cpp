#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <conio.h>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <numeric>
#include <intrin.h>

#include "..\HPPD\utils.h"
#include "..\HPPD\payloads.h"
#include "..\HPPD\seed.h"

#pragma comment(lib, "Msimg32.lib")

// ============================================================
// HPPD Benchmark
// Measures effective FPS and frame-time statistics for each
// payload by sampling GetTickCount at 16 ms intervals while
// the payload runs in its own thread.
// ============================================================

struct FrameStats
{
    double minMs = 1000.0;
    double maxMs = 0.0;
    double avgMs = 0.0;
    double p50Ms = 0.0;
    double p99Ms = 0.0;
    int sampleCount = 0;
};

static FrameStats RunPayloadBenchmark(
    LPTHREAD_START_ROUTINE payload,
    const wchar_t* name,
    int durationMs = 5000)
{
    printf("Benchmarking %ls... ", name);
    fflush(stdout);

    // Initialize the same way the main app does.
    gSeed =
        static_cast<uint32_t>(GetTickCount64())
        ^ static_cast<uint32_t>(__rdtsc());

    std::vector<double> sampleTimes;
    sampleTimes.reserve(800);

    // Run the payload in a worker thread.
    HANDLE payloadThread =
        CreateThread(NULL, 0, payload, NULL, 0, NULL);

    if (!payloadThread)
    {
        printf("FAILED (thread)\n");
        return {};
    }

    ULONGLONG start = GetTickCount64();
    ULONGLONG lastSample = start;
    ULONGLONG sample = 0;
    bool firstSample = true;

    while ((GetTickCount64() - start) < static_cast<ULONGLONG>(durationMs))
    {
        // Wait for the next 16 ms tick.
        ULONGLONG target = lastSample + 16;
        ULONGLONG now = GetTickCount64();
        if (now < target)
        {
            Sleep(static_cast<DWORD>(target - now));
            now = GetTickCount64();
        }

        double delta = static_cast<double>(now - lastSample);

        if (!firstSample)
            sampleTimes.push_back(delta);

        firstSample = false;
        lastSample = now;
        sample++;
    }

    // Let the payload finish naturally (its 5 s loop is
    // about to end, but ensure it stops).
    WaitForSingleObject(payloadThread, 2000);
    CloseHandle(payloadThread);

    if (sampleTimes.empty())
    {
        printf("FAILED (no samples)\n");
        return {};
    }

    FrameStats stats;
    stats.sampleCount = static_cast<int>(sampleTimes.size());

    stats.minMs = *std::min_element(sampleTimes.begin(), sampleTimes.end());
    stats.maxMs = *std::max_element(sampleTimes.begin(), sampleTimes.end());

    double sum = std::accumulate(sampleTimes.begin(), sampleTimes.end(), 0.0);
    stats.avgMs = sum / sampleTimes.size();

    std::sort(sampleTimes.begin(), sampleTimes.end());
    stats.p50Ms = sampleTimes[sampleTimes.size() / 2];
    stats.p99Ms = sampleTimes[static_cast<size_t>(sampleTimes.size() * 0.99)];

    printf("done\n");
    return stats;
}

static void PrintStats(const wchar_t* name, const FrameStats& s)
{
    printf("  %-32ls: samples=%4d  avg=%.2fms  p50=%.2fms  p99=%.2fms  min=%.2fms  max=%.2fms  effFPS=%.1f\n",
        name, s.sampleCount, s.avgMs, s.p50Ms, s.p99Ms, s.minMs, s.maxMs, 1000.0 / s.avgMs);
}

int main()
{
    printf("===============================================\n");
    printf("         HPPD Payload Performance Benchmark\n");
    printf("===============================================\n\n");

    printf("Screen: %dx%d\n",
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN));

    printf("Press any key to start...\n");
    _getch();

    struct PayloadInfo { const wchar_t* name; LPTHREAD_START_ROUTINE func; };
    PayloadInfo payloads[] = {
        { L"13 - Chromatic Plasma",        GDIPayload13 },
        { L"14 - RGB Displacement",        GDIPayload14 },
        { L"15 - Feedback Tunnel",         GDIPayload15 },
        { L"16 - Fractured Screen",        GDIPayload16 },
        { L"17 - Neon Edge Inversion",     GDIPayload17 },
        { L"18 - Liquid Ripple",           GDIPayload18 },
        { L"19 - Quantum Spiral",          GDIPayload19 },
        { L"20 - Color Worm",              GDIPayload20 },
        { L"21 - Radial Shockwave",        GDIPayload21 },
        { L"22 - RGB Void",                GDIPayload22 },
        { L"23 - Retro CRT",               GDIPayload23 },
        { L"24 - Absolute GDI Chaos",      GDIPayload24 },
    };

    const int count =
        static_cast<int>(sizeof(payloads) / sizeof(payloads[0]));

    printf("\nRunning %d payload benchmarks (5s each)...\n\n", count);

    std::vector<std::pair<const wchar_t*, FrameStats>> results;
    results.reserve(count);

    for (int i = 0; i < count; ++i)
    {
        FrameStats stats =
            RunPayloadBenchmark(payloads[i].func, payloads[i].name, 5000);

        if (stats.sampleCount > 0)
            results.emplace_back(payloads[i].name, stats);

        // Restore the desktop between payloads.
        RedrawWindow(
            NULL, NULL, NULL,
            RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);

        Sleep(500);
    }

    printf("\n===============================================\n");
    printf("                    RESULTS\n");
    printf("===============================================\n\n");

    // Sort fastest (lowest avg ms) first.
    std::sort(results.begin(), results.end(),
        [](const auto& a, const auto& b) {
            return a.second.avgMs < b.second.avgMs;
        });

    for (auto& r : results)
        PrintStats(r.first, r.second);

    printf("\n===============================================\n");
    printf("Press any key to exit...\n");
    _getch();

    return 0;
}