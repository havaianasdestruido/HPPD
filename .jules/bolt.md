## 2024-10-10 - Extracted invariant calculations in GDIPayload59 loops
**Learning:** Nested loops in rendering code (like per-pixel effect `GDIPayload59`) may unnecessarily compute invariant properties multiple times (like `sx` and `sy` sources for each pixel based on trigonometric functions).
**Action:** Extract trigonometric calculations that only depend on the source index to an array precomputed before the width and height loops.
## 2024-10-11 - Extracted row-invariant calculations in GDIPayload6 and GDIPayload69 loops
**Learning:** Found more instances where calculations dependent solely on `x` and `time` were wastefully recomputed inside the inner `y` per-pixel loops. This seems to be a recurring performance anti-pattern across multiple visual effects payloads (e.g. `GDIPayload6`, `GDIPayload69`).
**Action:** Always verify if inner-loop (typically `y`) mathematical operations (e.g. `sinf`, `cosf`) rely purely on invariant coordinates (like `x` or just `time`). Precalculate these into a `std::vector` outside the loop to dramatically save CPU cycles (by a factor of `h`).
