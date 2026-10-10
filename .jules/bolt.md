## 2024-10-10 - Extracted invariant calculations in GDIPayload59 loops
**Learning:** Nested loops in rendering code (like per-pixel effect `GDIPayload59`) may unnecessarily compute invariant properties multiple times (like `sx` and `sy` sources for each pixel based on trigonometric functions).
**Action:** Extract trigonometric calculations that only depend on the source index to an array precomputed before the width and height loops.
