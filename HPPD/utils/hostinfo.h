#pragma once

#include <string>

// ============================================================
// HostInfo: host reconnaissance reads, extracted from the rules
// engine so any payload (and rules.cpp itself) can reuse them.
//
// Every getter returns a ready-to-print AN *ANSI* string (best
// effort, never fails hard). Payloads that need wide text can
// use the W variants or ToWide().
// ============================================================

namespace HostInfo {

std::string KernelVersion();
std::string SystemInfo();
std::string MemoryInfo();
std::string LocalIpv4();
std::string InternetStatus();
std::string RoutingTable();
std::string TcpConnections();
std::string Hostname();
std::string DomainInfo();
std::string User();
std::string Integrity();
std::string Sid();
std::string ProcessInfo();
std::string ModuleInfo();
std::string ThreadInfo();
std::string RecentDocs();
std::string ServiceCount();
std::string AudioSrvStatus();
std::string TempFileStats();
std::string SelfFileStats();
std::string EventLogRecords();
std::string KeyboardLayout();
std::string DiskProperties();
std::string CdromStatus();
std::string DiskSize();
std::string GuiResources();
std::string UpTime();
std::string SystemLocale();
std::string UserLocale();
std::string BatteryStatus();
std::string ScreenSize();
std::string WindowsDir();
std::string TempDir();
std::string CursorPos();
std::string Clipboard();
std::string NetworkAdapters();

// Standard recon block: "key: value" lines for stdout.
void PrintAll();

// Wide variants for payloads that compose wide text
// (printers, window titles, ...).
std::wstring HostnameW();
std::wstring UserW();
std::wstring ToWide(const std::string& ansi);

}