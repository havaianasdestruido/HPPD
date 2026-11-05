// ============================================================
// fxcache_test - off-Windows test + benchmark for the RAM
// caches in gdi/fxcache.h and for the cache patterns used in
// gdi/heavy.cpp / gdi/light.cpp.
//
// The payload .cpp files need WinAPI, but the caches are pure
// math, so this harness re-implements the OLD per-pixel
// formulas next to the NEW cached ones and checks they agree
// (bit-exact where the cache stores the identical expression,
// which is most of them), then times both over many frames.
//
// Build & run (any C++14 compiler, no Windows needed):
//   g++ -O2 -std=c++14 -o fxcache_test tools/fxcache_test.cpp
//   ./fxcache_test
//
// Exit code 0 = every equivalence check passed.
// ============================================================

#include "../gdi/fxcache.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

static int gFailures = 0;

#define CHECK(cond, msg)                                        \
    do {                                                        \
        if (!(cond)) {                                          \
            std::printf("FAIL: %s (line %d)\n", msg, __LINE__); \
            ++gFailures;                                        \
        }                                                       \
    } while (0)

static bool SameBits(float a, float b)
{
    uint32_t ua, ub;
    std::memcpy(&ua, &a, sizeof(ua));
    std::memcpy(&ub, &b, sizeof(ub));
    return ua == ub;
}

static bool SameBits(double a, double b)
{
    uint64_t ua, ub;
    std::memcpy(&ua, &a, sizeof(ua));
    std::memcpy(&ub, &b, sizeof(ub));
    return ua == ub;
}

typedef std::chrono::steady_clock Clock;

static double MsSince(Clock::time_point t0)
{
    return std::chrono::duration<double, std::milli>(
        Clock::now() - t0).count();
}

// ------------------------------------------------------------
// 1. PolarMap stores exactly what the old inline code computed.
// ------------------------------------------------------------
static void TestPolarMap()
{
    const int sizes[][2] = { { 1, 1 }, { 3, 2 }, { 64, 48 }, { 191, 107 } };

    for (const auto& sz : sizes)
    {
        const int w = sz[0], h = sz[1];
        const float cx = w * 0.5f, cy = h * 0.5f;

        FxCache::PolarMap polar;
        CHECK(polar.Build(w, h, cx, cy), "PolarMap::Build");

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const float dx = x - cx;
                const float dy = y - cy;
                const size_t i = static_cast<size_t>(y) * w + x;

                CHECK(SameBits(polar.radius[i], sqrtf(dx * dx + dy * dy)),
                      "polar radius bit-exact");
                CHECK(SameBits(polar.angle[i], atan2f(dy, dx)),
                      "polar angle bit-exact");
                CHECK(SameBits(polar.cosA[i], cosf(atan2f(dy, dx))),
                      "polar cosA bit-exact");
                CHECK(SameBits(polar.sinA[i], sinf(atan2f(dy, dx))),
                      "polar sinA bit-exact");
            }
        }

        // Radius-only build must not materialise the other maps.
        FxCache::PolarMap rOnly;
        CHECK(rOnly.Build(w, h, cx, cy, FxCache::PolarRadius),
              "PolarMap radius-only build");
        CHECK(rOnly.angle.empty() && rOnly.cosA.empty() && rOnly.sinA.empty(),
              "radius-only build leaves other maps empty");
        CHECK(rOnly.radius.size() == static_cast<size_t>(w) * h,
              "radius-only map size");
    }

    std::printf("[ok] PolarMap: bit-exact vs inline sqrtf/atan2f/cosf/sinf\n");
}

// ------------------------------------------------------------
// 2. Bayer4Lut reproduces Payload 71's quantizer lambda.
// ------------------------------------------------------------
static int RefBayerQ(int v, int th)
{
    int c = v >> 6;

    if (c > 3)
        c = 3;

    if ((v & 63) > th)
        ++c;

    if (c > 3)
        c = 3;

    return c * 85;
}

static void TestBayerLut()
{
    const FxCache::Bayer4Lut lut = FxCache::BuildBayer4Lut();

    for (int thIndex = 0; thIndex < 16; ++thIndex)
        for (int v = 0; v < 256; ++v)
            CHECK(lut.level[thIndex][v] == RefBayerQ(v, thIndex * 4),
                  "Bayer LUT matches reference quantizer");

    std::printf("[ok] Bayer4Lut: all 4096 (threshold, value) results match\n");
}

// ------------------------------------------------------------
// 3. Payload 7 plasma: cached terms == original per-pixel terms.
// ------------------------------------------------------------
static void TestPlasmaCache()
{
    const int w = 160, h = 90;
    const int phase = 1234, amp = 5;

    std::vector<double> sinX(w), sinY(h), sinDiag(w + h);
    std::vector<double> sinRadial(static_cast<size_t>(w) * h);

    for (int x = 0; x < w; ++x)
        sinX[x] = std::sin((x + phase) / (16.0 - amp));
    for (int y = 0; y < h; ++y)
        sinY[y] = std::sin((y + phase) / (8.0 - amp / 2));
    for (int s = 0; s < w + h; ++s)
        sinDiag[s] = std::sin((s + phase) / 16.0);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            sinRadial[static_cast<size_t>(y) * w + x] =
                std::sin(std::sqrt(static_cast<double>(x * x + y * y)) /
                         (8.0 - amp / 2));

    for (int t = 0; t < 8; ++t)
    {
        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const int i = y * w + x;

                const double oldFx =
                    (t + t * std::sin((x + phase) / (16.0 - amp)) +
                     t + t * std::sin((y + phase) / (8.0 - amp / 2)) +
                     t + t * std::sin((x + y + phase) / 16.0) +
                     t + t * std::sin(std::sqrt(static_cast<double>(x * x + y * y)) /
                                      (8.0 - amp / 2))) / 4.0;

                const double newFx =
                    (t + t * sinX[x] +
                     t + t * sinY[y] +
                     t + t * sinDiag[x + y] +
                     t + t * sinRadial[i]) / 4.0;

                CHECK(SameBits(oldFx, newFx), "plasma cached fx bit-exact");
            }
        }
    }

    std::printf("[ok] Payload 7 plasma cache: bit-exact over 8 frames\n");
}

// ------------------------------------------------------------
// 4. Payload 54 Voronoi: cell trig tables == inline trig.
// ------------------------------------------------------------
static void TestVoronoiTables()
{
    const int w = 200, h = 120, cell = 80;

    std::vector<float> colFx(w), colGx(w), rowFy(h), rowGy(h);
    for (int x = 0; x < w; ++x) { colFx[x] = x / (float)cell; colGx[x] = floorf(colFx[x]); }
    for (int y = 0; y < h; ++y) { rowFy[y] = y / (float)cell; rowGy[y] = floorf(rowFy[y]); }

    const int maxCellX = static_cast<int>(colGx[w - 1]);
    const int maxCellY = static_cast<int>(rowGy[h - 1]);
    std::vector<float> sinCellY(maxCellY + 3), cosCellX(maxCellX + 3);

    for (int frame = 0; frame < 5; ++frame)
    {
        const float t = frame * .035f;

        for (int c = -1; c <= maxCellY + 1; ++c)
            sinCellY[c + 1] = sinf(static_cast<float>(c) + t);
        for (int c = -1; c <= maxCellX + 1; ++c)
            cosCellX[c + 1] = cosf(static_cast<float>(c) - t);

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const float fx = colFx[x], fy = rowFy[y];
                const float gx = colGx[x], gy = rowGy[y];
                float bestOld = 999999.0f, bestNew = 999999.0f;

                for (int oy = -1; oy <= 1; ++oy)
                {
                    for (int ox = -1; ox <= 1; ++ox)
                    {
                        const float pxO = gx + ox + .5f * sinf(gy + oy + t);
                        const float pyO = gy + oy + .5f * cosf(gx + ox - t);
                        const float pxN = gx + ox + .5f * sinCellY[static_cast<int>(gy) + oy + 1];
                        const float pyN = gy + oy + .5f * cosCellX[static_cast<int>(gx) + ox + 1];

                        CHECK(SameBits(pxO, pxN), "voronoi px bit-exact");
                        CHECK(SameBits(pyO, pyN), "voronoi py bit-exact");

                        const float dxO = fx - pxO, dyO = fy - pyO;
                        const float dxN = fx - pxN, dyN = fy - pyN;
                        bestOld = std::min(bestOld, dxO * dxO + dyO * dyO);
                        bestNew = std::min(bestNew, dxN * dxN + dyN * dyN);
                    }
                }

                CHECK(SameBits(bestOld, bestNew), "voronoi best bit-exact");
            }
        }
    }

    std::printf("[ok] Payload 54 cell tables: bit-exact over 5 frames\n");
}

// ------------------------------------------------------------
// 5. Payload 8 Mandelbrot: separable cx/cy + iteration map.
// ------------------------------------------------------------
static int MandelIter(double cx, double cy)
{
    double zx = 0, zy = 0;
    int iterations = 0;

    while (zx * zx + zy * zy < 10.0 && iterations < 35)
    {
        const double nx = zx * zx - zy * zy + cx;
        const double ny = 2.0 * zx * zy + cy;
        zx = nx; zy = ny;
        ++iterations;
    }

    return iterations;
}

static void TestMandelCache()
{
    const int w = 120, h = 80, phase = 777;

    const double fractalX = 2.5 / w;
    const double fractalY = 1.90 / h;

    std::vector<double> colX(w), rowY(h);
    for (int x = 0; x < w; ++x) colX[x] = x * fractalX - (2.0 - phase * 0.0001);
    for (int y = 0; y < h; ++y) rowY[y] = y * fractalY - (0.95 - phase * 0.0001);

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            CHECK(MandelIter(colX[x], rowY[y]) ==
                  MandelIter(x * fractalX - (2.0 - phase * 0.0001),
                             y * fractalY - (0.95 - phase * 0.0001)),
                  "mandelbrot cached iteration matches direct");

    std::printf("[ok] Payload 8 iteration cache: identical iteration counts\n");
}

// ------------------------------------------------------------
// Benchmarks: old per-pixel math vs RAM-cached math.
// Only the math kernels are timed (no GDI), checksums keep the
// compiler from optimising the loops away.
// ------------------------------------------------------------
static void BenchPlasma()
{
    const int w = 1280, h = 720, frames = 20;
    const int phase = 1234, amp = 5;
    volatile double sink = 0;

    auto t0 = Clock::now();
    for (int frame = 0; frame < frames; ++frame)
    {
        const int t = frame;
        double acc = 0;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const double fx =
                    (t + t * std::sin((x + phase) / (16.0 - amp)) +
                     t + t * std::sin((y + phase) / (8.0 - amp / 2)) +
                     t + t * std::sin((x + y + phase) / 16.0) +
                     t + t * std::sin(std::sqrt(static_cast<double>(x * x + y * y)) /
                                      (8.0 - amp / 2))) / 4.0;
                acc += static_cast<int>(std::sin(fx * 0.02) * 50.0);
            }
        sink = acc;
    }
    const double oldMs = MsSince(t0);

    std::vector<double> sinX(w), sinY(h), sinDiag(w + h);
    std::vector<double> sinRadial(static_cast<size_t>(w) * h);
    for (int x = 0; x < w; ++x) sinX[x] = std::sin((x + phase) / (16.0 - amp));
    for (int y = 0; y < h; ++y) sinY[y] = std::sin((y + phase) / (8.0 - amp / 2));
    for (int s = 0; s < w + h; ++s) sinDiag[s] = std::sin((s + phase) / 16.0);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            sinRadial[static_cast<size_t>(y) * w + x] =
                std::sin(std::sqrt(static_cast<double>(x * x + y * y)) / (8.0 - amp / 2));

    t0 = Clock::now();
    for (int frame = 0; frame < frames; ++frame)
    {
        const int t = frame;
        double acc = 0;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const int i = y * w + x;
                const double fx =
                    (t + t * sinX[x] + t + t * sinY[y] +
                     t + t * sinDiag[x + y] + t + t * sinRadial[i]) / 4.0;
                acc += static_cast<int>(std::sin(fx * 0.02) * 50.0);
            }
        sink = acc;
    }
    const double newMs = MsSince(t0);

    std::printf("[bench] plasma  %dx%d x%d frames: old %8.1f ms  cached %8.1f ms  (%.1fx, checksum %lld)\n",
                w, h, frames, oldMs, newMs, oldMs / newMs, (long long)sink);
}

static void BenchVortex()
{
    const int w = 960, h = 540, frames = 20;
    const float cx = w * 0.5f, cy = h * 0.5f;
    volatile int sink = 0;

    auto t0 = Clock::now();
    for (int frame = 0; frame < frames; ++frame)
    {
        const float time = frame * 0.035f;
        int acc = 0;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const float dx = x - cx, dy = y - cy;
                const float radius = sqrtf(dx * dx + dy * dy);
                const float angle = atan2f(dy, dx);
                const float vortex = sinf(radius * 0.012f - time * 4.0f) * 0.14f;
                const float a = angle + vortex;
                acc += static_cast<int>(cx + cosf(a) * radius) +
                       static_cast<int>(cx + cosf(a + 0.035f) * radius) +
                       static_cast<int>(cy + sinf(a - 0.035f) * radius);
            }
        sink = acc;
    }
    const double oldMs = MsSince(t0);

    FxCache::PolarMap polar;
    polar.Build(w, h, cx, cy, FxCache::PolarRadius | FxCache::PolarAngle);
    const float cosOff = cosf(0.035f), sinOff = sinf(0.035f);

    t0 = Clock::now();
    for (int frame = 0; frame < frames; ++frame)
    {
        const float time = frame * 0.035f;
        int acc = 0;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const size_t i = static_cast<size_t>(y) * w + x;
                const float radius = polar.radius[i];
                const float vortex = sinf(radius * 0.012f - time * 4.0f) * 0.14f;
                const float a = polar.angle[i] + vortex;
                const float ca = cosf(a), sa = sinf(a);
                const float caP = ca * cosOff - sa * sinOff;
                const float saM = sa * cosOff - ca * sinOff;
                acc += static_cast<int>(cx + ca * radius) +
                       static_cast<int>(cx + caP * radius) +
                       static_cast<int>(cy + saM * radius);
            }
        sink = acc;
    }
    const double newMs = MsSince(t0);

    std::printf("[bench] vortex  %dx%d x%d frames: old %8.1f ms  cached %8.1f ms  (%.1fx, checksum %d)\n",
                w, h, frames, oldMs, newMs, oldMs / newMs, (int)sink);
}

static void BenchMandel()
{
    const int w = 640, h = 360, frames = 30;
    volatile int sink = 0;

    auto t0 = Clock::now();
    for (int frame = 0; frame < frames; ++frame)
    {
        int acc = 0;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                acc += MandelIter(x * (2.5 / w) - 2.0, y * (1.90 / h) - 0.95) * 4;
        sink = acc;
    }
    const double oldMs = MsSince(t0);

    std::vector<uint8_t> iterMap(static_cast<size_t>(w) * h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            iterMap[static_cast<size_t>(y) * w + x] =
                static_cast<uint8_t>(MandelIter(x * (2.5 / w) - 2.0, y * (1.90 / h) - 0.95));

    t0 = Clock::now();
    for (int frame = 0; frame < frames; ++frame)
    {
        int acc = 0;
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
                acc += iterMap[static_cast<size_t>(y) * w + x] * 4;
        sink = acc;
    }
    const double newMs = MsSince(t0);

    std::printf("[bench] mandel  %dx%d x%d frames: old %8.1f ms  cached %8.1f ms  (%.1fx, checksum %d)\n",
                w, h, frames, oldMs, newMs, oldMs / newMs, (int)sink);
}

static void BenchPayload56()
{
    const int w = 1280, h = 720, frames = 30;
    const float cx = w * 0.5f, cy = h * 0.5f;
    volatile float sink = 0;

    std::vector<float> polar_radius(w * h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            polar_radius[y * w + x] = sqrtf((x - cx) * (x - cx) + (y - cy) * (y - cy));

    auto t0 = Clock::now();
    for (int frame = 0; frame < frames; ++frame)
    {
        float t = frame * 0.055f;
        float acc = 0;
        for (int i = 0; i < w * h; ++i)
            acc += 1.0f + sinf(polar_radius[i] * 0.02f - t * 4.0f) * 0.035f;
        sink = acc;
    }
    const double oldMs = MsSince(t0);

    std::vector<float> sinR(w * h), cosR(w * h);
    for (int i = 0; i < w * h; ++i)
    {
        sinR[i] = sinf(polar_radius[i] * 0.02f);
        cosR[i] = cosf(polar_radius[i] * 0.02f);
    }

    t0 = Clock::now();
    for (int frame = 0; frame < frames; ++frame)
    {
        float t4 = frame * 0.055f * 4.0f;
        float cosT = cosf(t4), sinT = sinf(t4);
        float acc = 0;
        for (int i = 0; i < w * h; ++i)
            acc += 1.0f + (sinR[i] * cosT - cosR[i] * sinT) * 0.035f;
        sink = acc;
    }
    const double newMs = MsSince(t0);

    std::printf("[bench] payload56 %dx%d x%d frames: old %8.1f ms  cached %8.1f ms  (%.1fx, checksum %d)\n",
                w, h, frames, oldMs, newMs, oldMs / newMs, (int)sink);
}

int main()
{
    TestPolarMap();
    TestBayerLut();
    TestPlasmaCache();
    TestVoronoiTables();
    TestMandelCache();

    // RAM footprint of a full-screen polar cache at 1080p.
    FxCache::PolarMap full;
    const auto t0 = Clock::now();
    full.Build(1920, 1080, 960.0f, 540.0f);
    const double buildMs = MsSince(t0);
    std::printf("[info] PolarMap 1920x1080: %.1f MB in RAM, built once in %.1f ms\n",
                full.Bytes() / (1024.0 * 1024.0), buildMs);

    BenchPlasma();
    BenchVortex();
    BenchMandel();
    BenchPayload56();

    if (gFailures == 0)
    {
        std::printf("ALL CHECKS PASSED\n");
        return 0;
    }

    std::printf("%d CHECK(S) FAILED\n", gFailures);
    return 1;
}
