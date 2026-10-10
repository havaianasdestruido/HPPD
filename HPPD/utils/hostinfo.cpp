#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h>
#include <windows.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <iphlpapi.h>
#include <wininet.h>
#include <sddl.h>
#undef interface // wininet/iphlpapi mess with this on some SDKs

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "hostinfo.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wininet.lib")

namespace HostInfo
{

namespace
{

std::string WideToUtf8(const wchar_t* w)
{
    if (!w || !w[0])
        return std::string();

    const int n = WideCharToMultiByte(
        CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);

    if (n <= 1)
        return std::string();

    std::string s(static_cast<size_t>(n - 1), '\0');

    WideCharToMultiByte(
        CP_UTF8, 0, w, -1, &s[0], n, nullptr, nullptr);

    return s;
}

typedef LONG(WINAPI* RtlGetVersion_t)(PRTL_OSVERSIONINFOW);

} // namespace

std::string KernelVersion()
{
    const RtlGetVersion_t rtl =
        reinterpret_cast<RtlGetVersion_t>(
            GetProcAddress(
                GetModuleHandleW(L"ntdll.dll"),
                "RtlGetVersion"
            )
        );

    if (!rtl)
        return "RtlGetVersion unavailable";

    OSVERSIONINFOW ovi = {};
    ovi.dwOSVersionInfoSize = sizeof(ovi);

    if (rtl(&ovi) == 0)
    {
        char buf[160];
        std::sprintf(
            buf,
            "Windows %lu.%lu build %lu (%ls)",
            (unsigned long)ovi.dwMajorVersion, (unsigned long)ovi.dwMinorVersion,
            (unsigned long)ovi.dwBuildNumber, ovi.szCSDVersion);
        return buf;
    }

    return "RtlGetVersion failed";
}

std::string SystemInfo()
{
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);

    char buf[128];
    std::sprintf(
        buf,
        "page %u, %u processors, arch %u",
        (unsigned int)si.dwPageSize,
        (unsigned int)si.dwNumberOfProcessors,
        (unsigned int)si.wProcessorArchitecture);
    return buf;
}

std::string MemoryInfo()
{
    MEMORYSTATUSEX ms = {};
    ms.dwLength = sizeof(ms);

    if (GlobalMemoryStatusEx(&ms))
    {
        char buf[160];
        std::sprintf(
            buf,
            "%llu MB total, %llu MB available",
            (unsigned long long)(ms.ullTotalPhys >> 20),
            (unsigned long long)(ms.ullAvailPhys >> 20));
        return buf;
    }

    return "GlobalMemoryStatusEx failed";
}

std::string LocalIpv4()
{
    ULONG size = 0;
    GetAdaptersAddresses(AF_INET, 0, nullptr, nullptr, &size);

    PIP_ADAPTER_ADDRESSES aa =
        static_cast<PIP_ADAPTER_ADDRESSES>(std::malloc(size));

    if (!aa)
        return "alloc failed";

    std::string out;
    int printed = 0;

    const ULONG rc =
        GetAdaptersAddresses(AF_INET, 0, nullptr, aa, &size);

    if (rc == NO_ERROR)
    {
        for (PIP_ADAPTER_ADDRESSES p = aa; p; p = p->Next)
        {
            for (PIP_ADAPTER_UNICAST_ADDRESS u = p->FirstUnicastAddress;
                 u; u = u->Next)
            {
                if (u->Address.lpSockaddr->sa_family != AF_INET)
                    continue;

                if (printed >= 3)
                    break;

                const sockaddr_in* sin =
                    reinterpret_cast<const sockaddr_in*>(u->Address.lpSockaddr);

                char line[256];
                std::sprintf(
                    line, "%ls -> %u.%u.%u.%u",
                    p->FriendlyName,
                    sin->sin_addr.S_un.S_un_b.s_b1,
                    sin->sin_addr.S_un.S_un_b.s_b2,
                    sin->sin_addr.S_un.S_un_b.s_b3,
                    sin->sin_addr.S_un.S_un_b.s_b4);

                if (!out.empty())
                    out += " | ";
                out += line;

                ++printed;
            }
        }
    }

    std::free(aa);

    if (printed == 0)
        return "no ipv4 adapter found";

    return out;
}

std::string InternetStatus()
{
    DWORD flags = 0;
    const bool online = InternetGetConnectedState(&flags, 0) != FALSE;

    return online
        ? "the internet is connected (say hi)"
        : "no internet, offline party";
}

std::string RoutingTable()
{
    PMIB_IPFORWARDTABLE table = nullptr;
    ULONG size = 0;

    GetIpForwardTable(nullptr, &size, TRUE);

    table = static_cast<PMIB_IPFORWARDTABLE>(std::malloc(size));

    if (table && GetIpForwardTable(table, &size, TRUE) == NO_ERROR)
    {
        char buf[128];
        std::sprintf(buf, "%lu routing entries in the table",
                     (unsigned long)table->dwNumEntries);
        std::free(table);
        return buf;
    }

    std::free(table);
    return "table read failed";
}

std::string TcpConnections()
{
    PMIB_TCPTABLE table = nullptr;
    ULONG size = 0;

    GetTcpTable(nullptr, &size, TRUE);

    table = static_cast<PMIB_TCPTABLE>(std::malloc(size));

    if (table && GetTcpTable(table, &size, TRUE) == NO_ERROR)
    {
        char buf[128];
        std::sprintf(buf, "%lu tcp connections visible (local), jokes only",
                     (unsigned long)table->dwNumEntries);
        std::free(table);
        return buf;
    }

    std::free(table);
    return "tcp table read failed";
}

std::wstring HostnameW()
{
    wchar_t name[MAX_COMPUTERNAME_LENGTH + 1] = {};
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

    GetComputerNameW(name, &size);
    return name;
}

std::string Hostname()
{
    const std::wstring w = HostnameW();
    return w.empty() ? "hostname unavailable" : WideToUtf8(w.c_str());
}

std::string DomainInfo()
{
    wchar_t name[256] = {};
    DWORD size = 256;

    GetComputerNameExW(ComputerNameDnsDomain, name, &size);

    return name[0] ? "member of a domain" : "workgroup machine, no domain";
}

std::wstring UserW()
{
    wchar_t user[128] = {};
    DWORD size = 128;

    GetUserNameW(user, &size);
    return user;
}

std::string User()
{
    const std::wstring w = UserW();
    return w.empty() ? "user name unavailable" : WideToUtf8(w.c_str());
}

std::string Integrity()
{
    HANDLE token = nullptr;

    if (!OpenProcessToken(
            GetCurrentProcess(),
            TOKEN_QUERY,
            &token))
        return "token open failed";

    DWORD size = 0;
    GetTokenInformation(
        token, TokenIntegrityLevel, nullptr, 0, &size);

    std::string out = "no integrity label";

    if (size > 0)
    {
        PTOKEN_MANDATORY_LABEL label = static_cast<PTOKEN_MANDATORY_LABEL>(
            std::malloc(size));

        if (label &&
            GetTokenInformation(
                token, TokenIntegrityLevel, label, size, &size))
        {
            PSID sid = label->Label.Sid;
            SID* sidData = static_cast<SID*>(sid);

            DWORD level = 0;
            if (sidData->SubAuthorityCount > 0)
                level = *(GetSidSubAuthority(sid, sidData->SubAuthorityCount - 1));

            const char* word =
                level <= 0x0FFF ? "Untrusted/Low" :
                level == 0x1000 ? "Low" :
                level == 0x2000 ? "Medium" :
                level == 0x3000 ? "High" :
                level == 0x4000 ? "System" :
                "weird";

            char buf[96];
            std::sprintf(buf, "integrity level %s (0x%X)", word, level);
            out = buf;
        }
        else
        {
            out = "token info read failed";
        }

        std::free(label);
    }

    CloseHandle(token);
    return out;
}

std::string Sid()
{
    HANDLE token = nullptr;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        return "token open failed";

    DWORD size = 0;
    GetTokenInformation(token, TokenUser, nullptr, 0, &size);

    std::string out = "sid read failed";

    if (size > 0)
    {
        PTOKEN_USER user = static_cast<PTOKEN_USER>(std::malloc(size));

        if (user &&
            GetTokenInformation(token, TokenUser, user, size, &size))
        {
            wchar_t* str = nullptr;

            if (ConvertSidToStringSidW(user->User.Sid, &str))
            {
                out = WideToUtf8(str);
                LocalFree(str);
            }
        }

        std::free(user);
    }

    CloseHandle(token);
    return out;
}

std::string ProcessInfo()
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe = {};
    pe.dwSize = sizeof(pe);

    if (snap == INVALID_HANDLE_VALUE)
        return "snapshot failed";

    int count = 0;
    wchar_t first[MAX_PATH] = {};

    if (Process32FirstW(snap, &pe))
    {
        do
        {
            ++count;
            if (count == 1)
                wcscpy(first, pe.szExeFile);
        } while (Process32NextW(snap, &pe));
    }

    CloseHandle(snap);

    char buf[256];
    std::sprintf(buf, "%d processes; first was %ls", count, first);
    return buf;
}

std::string ModuleInfo()
{
    HANDLE snap = CreateToolhelp32Snapshot(
        TH32CS_SNAPMODULE, GetCurrentProcessId());

    MODULEENTRY32W me = {};
    me.dwSize = sizeof(me);

    if (snap == INVALID_HANDLE_VALUE)
        return "snapshot failed";

    int count = 0;
    wchar_t first[MAX_PATH] = {};

    if (Module32FirstW(snap, &me))
    {
        do
        {
            ++count;
            if (count == 1)
                wcscpy(first, me.szModule);
        } while (Module32NextW(snap, &me));
    }

    CloseHandle(snap);

    char buf[256];
    std::sprintf(buf, "self has %d modules; base is %ls", count, first);
    return buf;
}

std::string ThreadInfo()
{
    HANDLE snap = CreateToolhelp32Snapshot(
        TH32CS_SNAPTHREAD, GetCurrentProcessId());

    THREADENTRY32 te = {};
    te.dwSize = sizeof(te);

    if (snap == INVALID_HANDLE_VALUE)
        return "snapshot failed";

    int count = 0;

    if (Thread32First(snap, &te))
    {
        do
        {
            if (te.th32OwnerProcessID == GetCurrentProcessId())
                ++count;
        } while (Thread32Next(snap, &te));
    }

    CloseHandle(snap);

    char buf[96];
    std::sprintf(buf, "%d threads in this process", count);
    return buf;
}

std::string RecentDocs()
{
    HKEY hKey = nullptr;

    const LONG rc =
        RegOpenKeyExW(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs",
            0,
            KEY_READ,
            &hKey);

    if (rc != ERROR_SUCCESS)
        return "RecentDocs not available";

    DWORD values = 0;
    RegQueryInfoKeyW(hKey, nullptr, nullptr, nullptr, nullptr, nullptr,
                     nullptr, &values, nullptr, nullptr, nullptr, nullptr);

    char buf[128];
    std::sprintf(buf, "RecentDocs has %lu values (last files opened)", (unsigned long)values);

    RegCloseKey(hKey);
    return buf;
}

std::string ServiceCount()
{
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);

    if (!scm)
        return "SCM connect failed (meh)";

    DWORD needed = 0;
    DWORD returned = 0;

    EnumServicesStatusExW(
        scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
        nullptr, 0, &needed, &returned, nullptr, nullptr);

    CloseServiceHandle(scm);

    char buf[128];
    std::sprintf(buf, "~%lu services exist on this box",
                 (unsigned long)(needed / sizeof(ENUM_SERVICE_STATUS_PROCESSW)));
    return buf;
}

std::string AudioSrvStatus()
{
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);

    if (!scm)
        return "SCM connect failed";

    SC_HANDLE svc = OpenServiceW(scm, L"AudioSrv", SERVICE_QUERY_STATUS);

    std::string out = "AudioSrv not found (XP+, eh)";

    if (svc)
    {
        SERVICE_STATUS ss = {};
        QueryServiceStatus(svc, &ss);

        char buf[96];
        std::sprintf(buf, "AudioSrv state = %lu", ss.dwCurrentState);
        out = buf;

        CloseServiceHandle(svc);
    }

    CloseServiceHandle(scm);
    return out;
}

void EnumFilesRecursively(const wchar_t* dir, int& files, long long& total)
{
    wchar_t pattern[MAX_PATH];
    wsprintfW(pattern, L"%s\\*", dir);

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern, &fd);

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (fd.cFileName[0] == L'.')
                continue;

            wchar_t sub[MAX_PATH];
            wsprintfW(sub, L"%s\\%s", dir, fd.cFileName);
            EnumFilesRecursively(sub, files, total);
        }
        else
        {
            ++files;
            total += (static_cast<long long>(fd.nFileSizeHigh) << 32) +
                     fd.nFileSizeLow;
        }
    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
}

std::string TempFileStats()
{
    wchar_t tmp[MAX_PATH];
    GetTempPathW(MAX_PATH, tmp);

    int files = 0;
    long long total = 0;
    EnumFilesRecursively(tmp, files, total);

    char buf[192];
    std::sprintf(
        buf,
        "%ls: %d files, %llu bytes",
        tmp, files, total);
    return buf;
}

std::string SelfFileStats()
{
    wchar_t self[MAX_PATH];
    GetModuleFileNameW(nullptr, self, MAX_PATH);

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(self, &fd);

    if (hFind == INVALID_HANDLE_VALUE)
        return "self find failed";

    char buf[224];
    std::sprintf(
        buf,
        "self: 0x%08lX attributes, %llu bytes of pure demoscene",
        fd.dwFileAttributes,
        (static_cast<long long>(fd.nFileSizeHigh) << 32) + fd.nFileSizeLow);

    FindClose(hFind);
    return buf;
}

std::string EventLogRecords()
{
    HANDLE log = OpenEventLogW(nullptr, L"System");

    if (!log)
        return "log open failed";

    DWORD records = 0;
    DWORD oldest = 0;

    std::string out = "log count failed";

    if (GetNumberOfEventLogRecords(log, &records))
    {
        char buf[96];
        std::sprintf(buf, "System log holds %lu records", (unsigned long)records);
        out = buf;
    }

    CloseEventLog(log);
    return out;
}

std::string KeyboardLayout()
{
    HKL list[8];
    const int n = GetKeyboardLayoutList(8, list);

    char buf[96];
    std::sprintf(buf, "%d keyboard layout(s); first handle 0x%p",
                 n, n > 0 ? list[0] : nullptr);
    return buf;
}

std::string DiskProperties()
{
    const DWORD mask = GetLogicalDrives();
    int drives = 0;
    int cdroms = 0;

    for (int i = 0; i < 26; ++i)
    {
        if (mask & (1u << i))
        {
            ++drives;

            char root[4] = { static_cast<char>('A' + i), ':', '\\', 0 };

            if (GetDriveTypeA(root) == DRIVE_CDROM)
                ++cdroms;
        }
    }

    char buf[128];
    std::sprintf(buf, "%d logical drives, %d of them CD/DVD",
                 drives, cdroms);
    return buf;
}

std::string CdromStatus()
{
    const int cdroms = [&]() -> int {
        const DWORD mask = GetLogicalDrives();
        int n = 0;

        for (int i = 0; i < 26; ++i)
        {
            if (mask & (1u << i))
            {
                char root[4] = { static_cast<char>('A' + i), ':', '\\', 0 };

                if (GetDriveTypeA(root) == DRIVE_CDROM)
                    ++n;
            }
        }

        return n;
    }();

    return cdroms == 0 ? "no optical drive (sad)" : "optical drive present";
}

std::string DiskSize()
{
    ULARGE_INTEGER total;
    total.QuadPart = 0;

    if (GetDiskFreeSpaceExW(L"C:\\", nullptr, &total, nullptr))
    {
        char buf[96];
        std::sprintf(buf, "C: total %.1f GB (we would love to fill it)",
                     static_cast<double>(total.QuadPart) / (1024.0 * 1024.0 * 1024.0));
        return buf;
    }

    return "C: probe failed";
}

std::string GuiResources()
{
    const DWORD gdi = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    const DWORD user = GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS);

    char buf[96];
    std::sprintf(buf, "GDI objects %lu, USER objects %lu", gdi, user);
    return buf;
}

std::string UpTime()
{
    const DWORD ms = GetTickCount();
    const DWORD days = ms / 86400000u;
    const DWORD rest = ms % 86400000u;
    const DWORD h = rest / 3600000u;
    const DWORD m = (rest % 3600000u) / 60000u;
    const DWORD s = (rest % 60000u) / 1000u;

    char buf[96];
    std::sprintf(buf, "uptime %lu day(s), %02lu:%02lu:%02lu",
                 (unsigned long)days, (unsigned long)h, (unsigned long)m, (unsigned long)s);
    return buf;
}

std::string SystemLocale()
{
    char buf[96];
    std::sprintf(buf, "system locale 0x%04lX",
                 static_cast<unsigned long>(GetSystemDefaultLCID()));
    return buf;
}

std::string UserLocale()
{
    char buf[96];
    std::sprintf(buf, "user locale 0x%04lX",
                 static_cast<unsigned long>(GetUserDefaultLCID()));
    return buf;
}

std::string BatteryStatus()
{
    SYSTEM_POWER_STATUS sps = {};
    if (!GetSystemPowerStatus(&sps))
        return "unknown (no power API)";

    char buf[96];
    if (sps.BatteryFlag == 255)
    {
        std::sprintf(buf, "no battery (desktop)");
    }
    else
    {
        std::sprintf(buf, "battery %lu%% %s",
                     (unsigned long)sps.BatteryLifePercent,
                     sps.ACLineStatus ? "on AC" : "on battery");
    }
    return buf;
}

std::string ScreenSize()
{
    char buf[96];
    std::sprintf(buf, "%ldx%ld primary, %ld monitor(s)",
                 GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN),
                 GetSystemMetrics(SM_CMONITORS));
    return buf;
}

std::string WindowsDir()
{
    wchar_t dir[MAX_PATH] = L"";
    GetWindowsDirectoryW(dir, MAX_PATH);
    return dir[0] ? WideToUtf8(dir) : "windows directory unavailable";
}

std::string TempDir()
{
    wchar_t dir[MAX_PATH] = L"";
    GetTempPathW(MAX_PATH, dir);
    return dir[0] ? WideToUtf8(dir) : "temp path unavailable";
}

std::string CursorPos()
{
    POINT pt = {};
    GetCursorPos(&pt);

    char buf[96];
    std::sprintf(buf, "cursor at (%ld, %ld)", pt.x, pt.y);
    return buf;
}

std::string Clipboard()
{
    if (!OpenClipboard(nullptr))
        return "clipboard busy/blocked";

    HANDLE h = GetClipboardData(CF_UNICODETEXT);

    std::string out = "clipboard empty or not text";

    if (h)
    {
        const wchar_t* txt = static_cast<const wchar_t*>(GlobalLock(h));
        if (txt)
        {
            char buf[96];
            std::sprintf(buf, "%zu char(s) on clipboard",
                         static_cast<size_t>(wcslen(txt)));
            out = buf;
            GlobalUnlock(h);
        }
        else
        {
            out = "cannot lock clipboard";
        }
    }

    CloseClipboard();
    return out;
}

std::string NetworkAdapters()
{
    ULONG size = 0;
    GetAdaptersInfo(nullptr, &size);
    if (size == 0)
        return "no adapter info";

    IP_ADAPTER_INFO* info = static_cast<IP_ADAPTER_INFO*>(std::malloc(size));
    if (!info)
        return "adapter query failed";

    std::string out = "adapter query failed";

    if (GetAdaptersInfo(info, &size) == NO_ERROR)
    {
        int n = 0;
        for (IP_ADAPTER_INFO* p = info; p; p = p->Next)
            ++n;

        char buf[196];
        std::sprintf(buf, "%d adapter(s); first '%s'", n, info->Description);
        out = buf;
    }

    std::free(info);
    return out;
}

std::wstring ToWide(const std::string& ansi)
{
    std::wstring out;
    out.resize(MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, nullptr, 0));

    if (!out.empty())
    {
        MultiByteToWideChar(
            CP_ACP, 0, ansi.c_str(), -1, &out[0],
            static_cast<int>(out.size()));
        out.resize(out.size() - 1); // drop the NUL
    }

    return out;
}

void PrintAll()
{
    struct Entry
    {
        const char* key;
        std::string (*fn)();
    };

    const Entry entries[] = {
        { "kernel",          KernelVersion   },
        { "system",          SystemInfo      },
        { "memory",          MemoryInfo      },
        { "ipv4",            LocalIpv4       },
        { "internet",        InternetStatus  },
        { "routes",          RoutingTable    },
        { "tcp",             TcpConnections  },
        { "hostname",        Hostname        },
        { "domain",          DomainInfo      },
        { "user",            User            },
        { "integrity",       Integrity       },
        { "sid",             Sid             },
        { "processes",       ProcessInfo     },
        { "modules",         ModuleInfo      },
        { "threads",         ThreadInfo      },
        { "recentdocs",      RecentDocs      },
        { "services",        ServiceCount    },
        { "audiosrv",        AudioSrvStatus  },
        { "temp",            TempFileStats   },
        { "self",            SelfFileStats   },
        { "eventlog",        EventLogRecords },
        { "keyboard",        KeyboardLayout  },
        { "disks",           DiskProperties  },
        { "cdrom",           CdromStatus     },
        { "c:",              DiskSize        },
        { "gui",             GuiResources    },
        { "uptime",          UpTime          },
        { "syslocale",       SystemLocale    },
        { "userlocale",      UserLocale      },
        { "battery",         BatteryStatus   },
        { "screen",          ScreenSize      },
        { "windows",         WindowsDir      },
        { "tempdir",         TempDir         },
        { "cursor",          CursorPos       },
        { "clipboard",       Clipboard       },
        { "adapters",        NetworkAdapters },
    };

    for (const Entry& e : entries)
        std::printf("[host] %-12s %s\n", e.key, e.fn().c_str());
}

} // namespace HostInfo