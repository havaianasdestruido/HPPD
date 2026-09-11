#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <cstdint>

// ============================================================
// HEAVY PAYLOADS
//
// Full-screen per-pixel renderers (fractals, Julia, reaction
// diffusion, ...). They share one slot (gHeavySlot in start.cpp)
// so only a single one paints the whole screen at a time.
//
//  1, 2, 5-11  original visual GDI effects.
//  13-24       fast visual GDI effects (5s, ESC aborts).
//  40, 48      ported GDI-effects dump.
//  50-63       GDI demoscene FX packs (per-pixel shaders).
// ============================================================

DWORD WINAPI GDIPayload1(LPVOID);
DWORD WINAPI GDIPayload2(LPVOID);
DWORD WINAPI GDIPayload5(LPVOID);
DWORD WINAPI GDIPayload6(LPVOID);
DWORD WINAPI GDIPayload7(LPVOID);
DWORD WINAPI GDIPayload8(LPVOID);
DWORD WINAPI GDIPayload9(LPVOID);
DWORD WINAPI GDIPayload10(LPVOID);
DWORD WINAPI GDIPayload11(LPVOID);
DWORD WINAPI GDIPayload13(LPVOID);
DWORD WINAPI GDIPayload14(LPVOID);
DWORD WINAPI GDIPayload15(LPVOID);
DWORD WINAPI GDIPayload17(LPVOID);
DWORD WINAPI GDIPayload19(LPVOID);
DWORD WINAPI GDIPayload20(LPVOID);
DWORD WINAPI GDIPayload21(LPVOID);
DWORD WINAPI GDIPayload22(LPVOID);
DWORD WINAPI GDIPayload23(LPVOID);
DWORD WINAPI GDIPayload24(LPVOID);
DWORD WINAPI GDIPayload40(LPVOID);
DWORD WINAPI GDIPayload48(LPVOID);
DWORD WINAPI GDIPayload50(LPVOID);
DWORD WINAPI GDIPayload51(LPVOID);
DWORD WINAPI GDIPayload52(LPVOID);
DWORD WINAPI GDIPayload53(LPVOID);
DWORD WINAPI GDIPayload54(LPVOID);
DWORD WINAPI GDIPayload55(LPVOID);
DWORD WINAPI GDIPayload56(LPVOID);
DWORD WINAPI GDIPayload57(LPVOID);
DWORD WINAPI GDIPayload58(LPVOID);
DWORD WINAPI GDIPayload59(LPVOID);
DWORD WINAPI GDIPayload60(LPVOID);
DWORD WINAPI GDIPayload61(LPVOID);
DWORD WINAPI GDIPayload62(LPVOID);
DWORD WINAPI GDIPayload63(LPVOID);