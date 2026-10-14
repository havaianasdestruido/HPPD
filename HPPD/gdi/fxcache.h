#pragma once

// ============================================================
// FX RAM CACHE
//
// The heavy (gdi/heavy.cpp) and light (gdi/light.cpp) payloads
// are per-pixel renderers: a 1920x1080 frame is ~2M pixels and
// each payload paints dozens to hundreds of frames. Recomputing
// the same sqrtf / atan2f / sinf / cosf / fractal iteration for
// every pixel of every frame burns CPU on results that either
// never change (they only depend on the pixel position and the
// per-run SEED variant) or only change along one axis / one
// diagonal of the image.
//
// This header holds the shared, RAM-resident caches used to
// stop that recomputation:
//
//   FxCache::PolarMap   Per-pixel radius / angle / unit-vector
//                       maps, built ONCE per payload run and
//                       read back every frame. Replaces the
//                       sqrtf + atan2f (+ cosf/sinf of the angle)
//                       that polar effects used to redo per
//                       pixel per frame.
//   FxCache::Bayer4Lut  Every (threshold, channel value) result
//                       of the Payload 71 ordered-dither
//                       quantizer: 16 x 256 bytes, built once,
//                       then a plain byte lookup per channel.
//
// Axis / diagonal tables that are specific to one formula
// (plasma terms, cell feature points, flow-field waves, Julia
// start points and iteration colours, ...) live next to their
// payload as plain std::vector<float/int> buffers allocated
// ONCE before the frame loop and refilled only when the frame
// time changes - see heavy.cpp / light.cpp for those.
//
// RAM cost at 1920x1080 (freed when the payload returns):
//   PolarMap (all parts)  16 bytes/pixel  ~ 33 MB
//   radius-only maps       4 bytes/pixel  ~  8 MB
//   Mandelbrot iter map    1 byte /pixel  ~  2 MB
// Axis/diagonal tables are w, h or w+h entries: negligible.
//
// Everything here is plain math (no WinAPI) so the caches can
// be unit-tested / benchmarked off Windows - see
// tools/fxcache_test.cpp.
// ============================================================

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace FxCache
{
    // Which per-pixel maps a PolarMap should materialise.
    // Building only the parts a payload reads keeps the RAM
    // footprint (and the one-time build cost) down.
    enum PolarParts : unsigned
    {
        PolarRadius = 1u << 0,  // sqrtf(dx*dx + dy*dy)
        PolarAngle  = 1u << 1,  // atan2f(dy, dx)
        PolarUnit   = 1u << 2,  // cosf(angle), sinf(angle)
        PolarAll    = PolarRadius | PolarAngle | PolarUnit
    };

    // Per-pixel polar coordinates around a centre (cx, cy),
    // indexed exactly like the pixel buffers: [y * w + x].
    //
    // The stored values are produced by the very same float
    // expressions the payloads used to evaluate inline
    // (sqrtf(dx*dx + dy*dy), atan2f(dy, dx), cosf/sinf of that
    // angle), so reading them back is bit-identical to
    // recomputing them - the CPU cost is just paid once.
    struct PolarMap
    {
        int w = 0;
        int h = 0;

        std::vector<float> radius;  // PolarRadius
        std::vector<float> angle;   // PolarAngle
        std::vector<float> cosA;    // PolarUnit: cosf(angle)
        std::vector<float> sinA;    // PolarUnit: sinf(angle)

        bool Build(
            int w_,
            int h_,
            float cx,
            float cy,
            unsigned parts = PolarAll
        )
        {
            w = w_;
            h = h_;

            if (w <= 0 || h <= 0)
                return false;

            const size_t count =
                static_cast<size_t>(w) * static_cast<size_t>(h);

            if (parts & PolarRadius)
                radius.resize(count);

            if (parts & PolarAngle)
                angle.resize(count);

            if (parts & PolarUnit)
            {
                cosA.resize(count);
                sinA.resize(count);
            }

            const bool wantR = (parts & PolarRadius) != 0;
            const bool wantA = (parts & PolarAngle) != 0;
            const bool wantU = (parts & PolarUnit) != 0;

            for (int y = 0; y < h; ++y)
            {
                const float dy = static_cast<float>(y) - cy;
                const size_t row = static_cast<size_t>(y) * w;

                for (int x = 0; x < w; ++x)
                {
                    const float dx = static_cast<float>(x) - cx;
                    const size_t i = row + static_cast<size_t>(x);

                    if (wantR)
                        radius[i] = sqrtf(dx * dx + dy * dy);

                    if (wantA || wantU)
                    {
                        const float a = atan2f(dy, dx);

                        if (wantA)
                            angle[i] = a;

                        if (wantU)
                        {
                            cosA[i] = cosf(a);
                            sinA[i] = sinf(a);
                        }
                    }
                }
            }

            return true;
        }

        // Total bytes held in RAM by this map.
        size_t Bytes() const
        {
            return (
                radius.size() +
                angle.size() +
                cosA.size() +
                sinA.size()
            ) * sizeof(float);
        }
    };

    // --------------------------------------------------------
    // Ordered-dither (Bayer 4x4) quantizer cache for the light
    // Payload 71: level[thresholdIndex][channelValue] holds the
    // exact result the per-pixel quantizer lambda used to
    // compute (2 bits/channel -> 0, 85, 170 or 255).
    // --------------------------------------------------------
    struct Bayer4Lut
    {
        uint8_t level[16][256];
    };

    static inline Bayer4Lut BuildBayer4Lut()
    {
        Bayer4Lut lut{};

        for (int thIndex = 0; thIndex < 16; ++thIndex)
        {
            const int th = thIndex * 4;

            for (int v = 0; v < 256; ++v)
            {
                int c = v >> 6;

                if (c > 3)
                    c = 3;

                if ((v & 63) > th)
                    ++c;

                if (c > 3)
                    c = 3;

                lut.level[thIndex][v] =
                    static_cast<uint8_t>(c * 85);
            }
        }

        return lut;
    }
}
