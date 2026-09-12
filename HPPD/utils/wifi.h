#pragma once

// ============================================================
// Native Wifi queries (wlanapi.dll, shipped with XP SP2+). The
// module opens the DLL by hand because the 7.1A SDK is too old
// to declare the API, and wlanapi is not import-lib linked here.
//
// All reads are non-destructive. Version negotiation talks WAPI2
// when available and falls back to WAPI v1 otherwise.
// ============================================================

namespace Wifi {

// Print SSID, signal %, auth (open/WPA/WPA2) and profile name for
// every visible network on every interface.
void ListNetworks();

// Print the machine's saved Wifi profiles (name, SSID and flags —
// never passwords, the demo is non-destructive).
void ListProfiles();

}