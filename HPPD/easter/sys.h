#pragma once

// ============================================================
// System-based easter egg.
//
// Looks for XP-era markers of a (likely) pirated Windows copy
// and, if any is found, mocks the user with a message box:
//
//   title: "hahahaha"
//   text : "looks like someones too poor to get its
//           computer fried."
//
// Markers inspected:
//   HKLM\...\WPAEvents -> OOBETimer    (activation timer)
//   HKLM\...\Winlogon\Notify\WgaLogon  (failed WGA check)
//   HKLM\...\CurrentVersion -> ProductID (FCKGW key block)
// ============================================================

void EasterSys();