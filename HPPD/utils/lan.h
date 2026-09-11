#pragma once

// ============================================================
// LAN / SMB queries and the classic net send prank.
//
// NetServerEnum + NetShareEnum behave on XP and W10 (SMB1 share
// browsing reads the browser service; on modern networks both
// calls may simply return 0 entries). NetMessageBufferSend only
// reaches XP/2003-era Messenger service — harmless no-op from
// Vista onward, kept for the emissary value.
// ============================================================

namespace Lan {

// Print every machine on the local workgroup domain.
void ListServers();

// Print every share offered by `server` (\\NAME).
void ListShares(const wchar_t* server);

// Print the ARP table for each active adapter.
void ArpNeighbors();

// Net send gag: send `text` to `target` (\c WS_...) — requires
// the Messenger service on the other side. Returns false on XP
// without the service or on any modern OS.
bool NetMessage(const wchar_t* target, const wchar_t* text);

}