## 2024-10-10 - Extracted invariant calculations in GDIPayload59 loops
**Learning:** Nested loops in rendering code (like per-pixel effect `GDIPayload59`) may unnecessarily compute invariant properties multiple times (like `sx` and `sy` sources for each pixel based on trigonometric functions).
**Action:** Extract trigonometric calculations that only depend on the source index to an array precomputed before the width and height loops.

## 2024-10-11 - Extracted row-invariant calculations in GDIPayload6 and GDIPayload69 loops
**Learning:** Found more instances where calculations dependent solely on `x` and `time` were wastefully recomputed inside the inner `y` per-pixel loops. This seems to be a recurring performance anti-pattern across multiple visual effects payloads (e.g. `GDIPayload6`, `GDIPayload69`).
**Action:** Always verify if inner-loop (typically `y`) mathematical operations (e.g. `sinf`, `cosf`) rely purely on invariant coordinates (like `x` or just `time`). Precalculate these into a `std::vector` outside the loop to dramatically save CPU cycles (by a factor of `h`).

## 2026-10-14 - Cached frame-invariant per-pixel math in RAM (fxcache)
**Learning:** The axis/diagonal extraction above still leaves the biggest costs untouched: many heavy/light payloads (`GDIPayload7`, `8`, `50`, `51`, `53`, `54`, `56`, `59`, `72`) recomputed position-only math - `sqrtf`/`atan2f` polar coordinates, a whole Mandelbrot iteration, Voronoi cell trig - for every pixel of every frame, even though the results never change during a run (they depend only on pixel position and the per-run SEED variant). Others (`17`, `23`, `55`, `62`, `63`) hid axis-, diagonal- or grid-only terms inside the per-pixel loop, and `GDIPayload71` re-ran a 16x256-case quantizer per channel.
**Action:** Build the expensive results ONCE before the frame loop and keep them in RAM for the payload's lifetime (shared `FxCache::PolarMap` + `FxCache::Bayer4Lut` in `gdi/fxcache.h`, plus per-payload `std::vector` iteration/axis/diagonal/cell tables allocated once, refilled only when they carry frame time). Cached values reuse the exact original expressions, so output stays bit-identical (the only exceptions are angle-addition identities in `GDIPayload50`/`51`, algebraically identical). Verified + benchmarked off-Windows in `tools/fxcache_test.cpp`: plasma kernel ~6.5x, vortex kernel ~2.1x, Mandelbrot colourise ~300x faster; full-screen polar map costs ~32 MB at 1080p and ~54 ms one-time build.

## 2026-10-16 - Batched random bit generation in GDIPayload70
**Learning:** Pixel shaders generating random noise (e.g. analog TV static) often call the RNG separately for each colour channel (R, G, B). Since our PRNG (FXRand) returns a 32-bit integer, we are wasting 24 bits of randomness per call.
**Action:** Fetch all three 8-bit components (r, g, b) from a single FXRand() call using bitwise masking and shifting. This cuts the number of RNG calls per pixel from 3 to 1.

## 2026-10-17 - Precomputed distance squares in GDIPayload22
**Learning:** Per-pixel distance calculations (like dx^2 + dy^2) inside nested loops waste CPU cycles since coordinates don't change between frames.
**Action:** Extract distance coordinate calculations (dx^2 and dy^2) to 1D vectors before the frame loops so that inner calculations become a single addition.

## 2026-10-18 - Batched random bit generation in GDIPayload74
**Learning:** Found another instance of the random bit generation anti-pattern in `GDIPayload74` where `FXRand()` was called 3 times per pixel to generate random static noise for the R, G, and B components. The same optimization from `GDIPayload70` applies.
**Action:** Replaced the 3 per-pixel `FXRand()` calls with a single call by extracting the three 8-bit components (r, g, b) using bitwise masking and shifting (`rnd & 255`, `(rnd >> 8) & 255`, `(rnd >> 16) & 255`).

## 2026-10-19 - Pre-caching trig functions and using angle-addition identity in GDIPayload50
**Learning:** In `GDIPayload50`, the `vortex` calculation in the per-pixel hot loop depended on `sinf(radius * 0.012f - time * 4.0f)`. Although `radius` was already cached via `polar.radius`, the `sinf` was still evaluated per pixel per frame.
**Action:** By extracting the `radius * 0.012f` terms into pre-calculated `sinR` and `cosR` arrays outside the frame loop, and applying the angle-addition identity `sin(A - B) = sin(A)cos(B) - cos(A)sin(B)` inside the loop, we avoid calling `sinf()` millions of times per frame, significantly improving performance (~1.8x faster measured by `fxcache_test`). This is a powerful pattern for any `sin(invariant +/- time)` expression inside hot loops.

## 2026-10-20 - Using angle-addition identity to separate row and column calculations in GDIPayload55
**Learning:** In `GDIPayload55`, inner per-pixel loops contained `sinf(colNx17[x] + rowSin[y])` and `sinf(rowNy13[y] - colCos[x])`. Although these expressions combine both row and column components, they can be decoupled using the trigonometric angle-addition identities `sin(A + B) = sin(A)cos(B) + cos(A)sin(B)` and `sin(A - B) = sin(A)cos(B) - cos(A)sin(B)`.
**Action:** Precalculate `sinf` and `cosf` for the individual row and column components outside the per-pixel loops. Then, use the angle-addition identity inside the loops to calculate the final sine value. This converts `O(w * h)` expensive trigonometric function calls into `O(w + h)` trig calls and some basic arithmetic inside the loop, yielding a ~2.2x speedup.
## 2026-10-21 - Extract frame-invariant terms using angle-addition identity in GDIPayload53
**Learning:** In `GDIPayload53`, per-pixel loops contained `sinf(r * .025f - t * 5.0f + sinA6[pi] * 2.0f)` and `sinf(r * .01f - t * 3.0f)`. These expressions re-calculate frame-invariant values millions of times per frame.
**Action:** Decouple these expressions using the trigonometric angle-addition identity `sin(A - B) = sin(A)cos(B) - cos(A)sin(B)`. Precompute `sinf` and `cosf` for the frame-invariant terms (`r * .025f + sinA6[pi] * 2.0f` and `r * .01f`) into arrays outside the frame loop. Then, apply the identity inside the loop with the time-varying components.
