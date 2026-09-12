#pragma once

// ============================================================
// Optical drive tray puppeteer via the MCI media-control
// interface (mciSendStringW). Present on every Windows; a machine
// without an optical drive will simply see failures.
// ============================================================

namespace CdrRom {

// Count of optical drives (GetDriveType == DRIVE_CDROM).
int Count();

// Eject / close the tray of the FIRST optical drive found.
bool Eject();
bool Close();

// Same for every optical drive, one by one.
void EjectAll();
void CloseAll();

// Open, wait `waitMs`, close.
void Prank(DWORD waitMs = 2500);

}