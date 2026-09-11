#pragma once

#include <windows.h>

// ============================================================
// Desktop shell interaction: recycle bin, shortcuts, folder
// helpers. All paths are SHGetKnownFolderPath where available
// (Vista+) with a SHGetFolderPathW fallback so XP still works.
// ============================================================

namespace ShellUtils {

// Size of the recycle bin in bytes, or 0 when empty.
ULONGLONG RecycleBinBytes();

// Empty the bin (permanent). true on success.
bool EmptyRecycleBin();

// Move `path` to the recycle bin instead of deleting it.
bool DeleteToBin(const wchar_t* path);

// Create a .lnk shortcut at linkPath pointing at target, with
// optional working directory and description.
bool CreateShortcut(const wchar_t* linkPath,
	const wchar_t* target,
	const wchar_t* workDir = nullptr,
	const wchar_t* description = nullptr);

// Create a tiny .url "internet shortcut" (text file, works on
// every OS).
bool CreateUrlShortcut(const wchar_t* linkPath, const wchar_t* url);

// Common folder paths (single buffer caller copies; these point
// into a static allocation).
const wchar_t* DesktopPath();
const wchar_t* StartupPath();

}