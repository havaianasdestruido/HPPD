#include "rules.h"

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <windows.h>
#include <wincrypt.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <iphlpapi.h>
#include <wininet.h>
#include <shellapi.h>
#include <shlobj.h>
#include <sddl.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "shell32.lib")

namespace Rules
{
    // ------------------------------------------------------------
    // Small helpers
    // ------------------------------------------------------------

    static void Report(const char* id, const char* verdict)
    {
        std::printf("[rule] %s -> %s\n", id, verdict);
    }

    static void W(const char* id, const wchar_t* wide, const char* fallback)
    {
        char buf[512];
        int n = WideCharToMultiByte(
            CP_UTF8, 0, wide, -1, buf, sizeof(buf), nullptr, nullptr);
        if (n > 0)
            Report(id, buf);
        else
            Report(id, fallback);
    }

    // Forward declaration of the gag thread entry.
    static DWORD WINAPI RunAllThread(LPVOID param);

    struct RuleDef
    {
        const char* id;
        void (*fn)();
    };

    // ------------------------------------------------------------
    // anti-analysis / anti-debugging
    // ------------------------------------------------------------

    static void CheckDebuggerViaApi()
    {
        BOOL remote = FALSE;
        CheckRemoteDebuggerPresent(GetCurrentProcess(), &remote);

        const bool hooked =
            IsDebuggerPresent() || remote != FALSE;

        Report(
            "check-for-debugger-via-api",
            hooked
                ? "debugger present (IsDebuggerPresent / CheckRemoteDebuggerPresent)"
                : "no debugger attached"
        );
    }

    static void CheckPebBeingDebugged()
    {
#if defined(_M_IX86)
        const DWORD peb = __readfsdword(0x30);
        const BYTE beingDebugged = *(reinterpret_cast<const BYTE*>(peb + 0x02));
        const DWORD ntGlobalFlag = *(reinterpret_cast<const DWORD*>(peb + 0x68));
#elif defined(_M_X64)
        const DWORD64 peb = __readgsqword(0x60);
        const BYTE beingDebugged = *(reinterpret_cast<const BYTE*>(peb + 0x02));
        const DWORD ntGlobalFlag = *(reinterpret_cast<const DWORD*>(peb + 0xBC));
#else
        const BYTE beingDebugged = IsDebuggerPresent() ? 1 : 0;
        const DWORD ntGlobalFlag = 0;
#endif

        if (beingDebugged)
            Report("check-for-peb-beingdebugged-flag", "PEB.BeingDebugged == 1 (debugger)");
        else
            Report("check-for-peb-beingdebugged-flag", "PEB.BeingDebugged == 0");

        const DWORD dbgFlags = 0x10 | 0x20 | 0x40;  // FLG_HEAP_ENABLE_TAIL_CHECK/FREE_CHECK/VALIDATE_PARAMETERS

        Report(
            "check-for-peb-ntglobalflag-flag",
            (ntGlobalFlag & dbgFlags) == dbgFlags
                ? "NtGlobalFlag carries debug heap flags"
                : "NtGlobalFlag clean"
        );
    }

    typedef LONG(WINAPI* NtQueryInformationProcess_t)(
        HANDLE, ULONG, PVOID, ULONG, PULONG);

    static void CheckProcessDebugPort()
    {
        const NtQueryInformationProcess_t qip =
            reinterpret_cast<NtQueryInformationProcess_t>(
                GetProcAddress(
                    GetModuleHandleW(L"ntdll.dll"),
                    "NtQueryInformationProcess"
                )
            );

        if (!qip)
        {
            Report("check-processdebugport", "ntdll symbol missing (won't happen)");
            return;
        }

        ULONG_PTR debugPort = 0;

        const LONG status =
            qip(
                GetCurrentProcess(),
                7,                       // ProcessDebugPort
                &debugPort,
                sizeof(debugPort),
                nullptr
            );

        Report(
            "check-processdebugport",
            (status == 0 && debugPort != 0)
                ? "valid debug port found"
                : "no debug port"
        );
    }

    static void CheckJobObject()
    {
        BOOL inJob = FALSE;
        IsProcessInJob(GetCurrentProcess(), nullptr, &inJob);

        Report(
            "check-process-job-object",
            inJob ? "process is inside a job object" : "not in a job object"
        );
    }

    static void CheckKernelDebugger()
    {
        // KUSER_SHARED_DATA legacy KdDebuggerEnabled (works XP..Win8,
        // later kernels moved the flag; read stays harmless).
        const BYTE kd = *reinterpret_cast<const BYTE*>(0x7FFE0000u + 0x2D4);

        Report(
            "check-for-kernel-debugger-via-shared-user-data-structure",
            kd ? "kernel debugger flag set" : "kernel debugger flag clear"
        );
    }

    static void CheckSoftwareBreakpoints()
    {
        const BYTE* base = reinterpret_cast<const BYTE*>(GetModuleHandleW(nullptr));
        int int3 = 0;

        for (int i = 0; i < 4096; ++i)
        {
            if (base[i] == 0xCC)
                ++int3;
        }

        char buf[128];
        std::sprintf(buf, "self-scan found %d int3 (0xCC) bytes in the first 4KB", int3);
        Report("check-for-software-breakpoints", buf);
    }

    static void CheckHardwareBreakpoints()
    {
        CONTEXT ctx;
        ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
        ctx.Dr0 = ctx.Dr1 = ctx.Dr2 = ctx.Dr3 = 0;
        ctx.Dr6 = ctx.Dr7 = 0;

        if (GetThreadContext(GetCurrentThread(), &ctx))
        {
            char buf[160];
            std::sprintf(
                buf,
                "DR0-3 = %08X %08X %08X %08X (DR7 %08X)",
                ctx.Dr0, ctx.Dr1, ctx.Dr2, ctx.Dr3, ctx.Dr7);
            Report("check-for-hardware-breakpoints", buf);
        }
        else
        {
            Report("check-for-hardware-breakpoints", "hardware registers hidden by the OS");
        }
    }

    static void CheckTimeDelayGetTickCount()
    {
        const DWORD t0 = GetTickCount();
        Sleep(50);
        const DWORD dt = GetTickCount() - t0;

        char buf[96];
        std::sprintf(buf, "50ms pause took %u ms on real time", dt);
        Report("check-for-time-delay-via-gettickcount", buf);
    }

    static void CheckTimeDelayQpc()
    {
        LARGE_INTEGER freq, t0, t1;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&t0);
        Sleep(25);
        QueryPerformanceCounter(&t1);

        const double ms =
            1000.0 * static_cast<double>(t1.QuadPart - t0.QuadPart) /
            static_cast<double>(freq.QuadPart);

        char buf[96];
        std::sprintf(buf, "25ms pause took %.2f ms on QPC", ms);
        Report("check-for-time-delay-via-queryperformancecounter", buf);
    }

    static void CheckOutputDebugString()
    {
        OutputDebugStringW(L"HPPD rules probe");
        Report(
            "check-for-outputdebugstring-error",
            GetLastError() == 0 ? "OutputDebugString returned clean" : "OutputDebugString tripped"
        );
    }

    static void ExecuteAntiDebugInstructions()
    {
        // The rule calls for executing anti-debug instructions (int 3,
        // trap flags, ...). It's a joke: we just take over the "PEB" and
        // report instead of actually tripping a debugger.
        Report(
            "execute-anti-debugging-instructions",
            "wanted to int3 / set trap flag; declined (jokes only)"
        );
    }

    static void CheckForegroundWindowSwitch()
    {
        const HWND a = GetForegroundWindow();
        Sleep(300);
        const HWND b = GetForegroundWindow();

        Report(
            "check-for-foreground-window-switch",
            a != b ? "foreground window changed (interactive)" : "foreground window static"
        );
    }

    static void CheckUnmovingMouse()
    {
        POINT a, b;
        GetCursorPos(&a);
        Sleep(450);
        GetCursorPos(&b);

        Report(
            "check-for-unmoving-mouse-cursor",
            (a.x != b.x || a.y != b.y)
                ? "cursor moved (a human is here)"
                : "cursor did not move (sandbox sleeps?)"
        );
    }

    static void CheckSandboxUsernameOrHostname()
    {
        static const wchar_t* const suspects[] = {
            L"sandbox", L"sample", L"malsandbox", L"malware",
            L"test", L"user", L"vmware", L"vm", L"win-xp",
        };

        wchar_t name[128] = {};
        wchar_t user[128] = {};
        DWORD size = 128;

        GetComputerNameW(name, &size);

        size = 128;
        GetUserNameW(user, &size);

        for (const wchar_t* s : suspects)
        {
            if (wcsstr(name, s) || wcsstr(user, s))
            {
                Report(
                    "check-for-sandbox-username-or-hostname",
                    "hostname/user smells like a sandbox (joke)"
                );
                return;
            }
        }

        Report("check-for-sandbox-username-or-hostname", "hostname/user look human");
    }

    static void CheckWindowsSandboxViaProcessName()
    {
        static const wchar_t* const sandboxProcs[] = {
            L"WindowsSandboxClient.exe",
            L"WindowsSandbox.exe",
            L"vmsnim.exe",
        };

        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32W pe = {};
        pe.dwSize = sizeof(pe);

        if (snap == INVALID_HANDLE_VALUE)
        {
            Report("check-for-windows-sandbox-via-process-name", "snapshot failed");
            return;
        }

        bool hit = false;

        if (Process32FirstW(snap, &pe))
        {
            do
            {
                for (const wchar_t* s : sandboxProcs)
                {
                    if (_wcsicmp(pe.szExeFile, s) == 0)
                    {
                        hit = true;
                        break;
                    }
                }
            } while (!hit && Process32NextW(snap, &pe));
        }

        CloseHandle(snap);

        Report(
            "check-for-windows-sandbox-via-process-name",
            hit ? "windows sandbox process found" : "no windows sandbox process"
        );
    }

    static void CheckSandboxAndAvModules()
    {
        // Detection-style rule: enumerate loaded modules and look for
        // known sandbox / AV helper DLL names.
        static const wchar_t* const avDlls[] = {
            L"sbiedll.dll",        // Sandboxie
            L"snxhk.dll",          // Avast
            L"ekrn",               // ESET
            L"kis",
            L"avp.dll",            // Kaspersky
            L"VBoxHook.dll",
        };

        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
        MODULEENTRY32W me = {};
        me.dwSize = sizeof(me);

        if (snap == INVALID_HANDLE_VALUE)
        {
            Report("check-for-sandbox-and-av-modules", "module snapshot failed");
            return;
        }

        bool av = false;
        int count = 0;

        if (Module32FirstW(snap, &me))
        {
            do
            {
                ++count;

                for (const wchar_t* s : avDlls)
                {
                    if (wcsstr(me.szModule, s))
                    {
                        av = true;
                        break;
                    }
                }
            } while (Module32NextW(snap, &me));
        }

        CloseHandle(snap);

        char buf[128];
        std::sprintf(
            buf,
            "%d modules; %s",
            count,
            av ? "sandbox/AV helper module loaded" : "no sandbox/AV helper module");
        Report("check-for-sandbox-and-av-modules", buf);
    }

    static void ReferenceAntiVmStrings()
    {
        // Scan our own image for classic anti-VM marker strings.
        static const char* const markers[] = {
            "VBoxGuest", "VMware", "vmmouse", "QEMU", "Bochs",
        };

        const BYTE* base = reinterpret_cast<const BYTE*>(GetModuleHandleW(nullptr));
        const BYTE* end = base + 1024 * 1024;

        int hits = 0;

        for (const char* m : markers)
        {
            const size_t len = std::strlen(m);

            for (const BYTE* p = base; p + len < end; ++p)
            {
                if (std::memcmp(p, m, len) == 0)
                {
                    ++hits;
                    break;
                }
            }
        }

        char buf[128];
        std::sprintf(buf, "own image contains %d anti-VM string markers", hits);
        Report("reference-anti-vm-strings", buf);
    }

    static void TerminateAnthropicSessionViaMagicStrings()
    {
        // The two magic strings are embedded in the binary (see main.cpp
        // EasterEggMagicStrings). Embedding them IS what this rule looks
        // for, so the rule firing is post-hoc: strings live, session dies.
        Report(
            "terminate-anthropic-session-via-magic-strings",
            "magic strings embedded; ANTHROPIC session terminated (joke)"
        );
    }

    static void PackedWithSomething()
    {
        static const char* const packers[] = {
            "themida", "vmprotect", "upx", "aspack", "petite",
            "pecompact", "nspack", "y0da", "rlpack", "upack",
        };

        const char* pick = packers[(GetTickCount() / 1000) % 10];

        char buf[96];
        std::sprintf(buf, "binary allegedly packed with %s (it isn't)", pick);
        Report("packed-with-generic-packer", buf);
    }

    // ------------------------------------------------------------
    // host-interaction (reads + reversible plays)
    // ------------------------------------------------------------

    typedef LONG(WINAPI* RtlGetVersion_t)(PRTL_OSVERSIONINFOW);

    static void GetKernelVersion()
    {
        const RtlGetVersion_t rtl =
            reinterpret_cast<RtlGetVersion_t>(
                GetProcAddress(
                    GetModuleHandleW(L"ntdll.dll"),
                    "RtlGetVersion"
                )
            );

        if (!rtl)
        {
            Report("get-kernel-version", "RtlGetVersion unavailable");
            return;
        }

        OSVERSIONINFOW ovi = {};
        ovi.dwOSVersionInfoSize = sizeof(ovi);

        if (rtl(&ovi) == 0)
        {
            char buf[160];
            std::sprintf(
                buf,
                "Windows %lu.%lu build %lu (%ls)",
                ovi.dwMajorVersion, ovi.dwMinorVersion,
                ovi.dwBuildNumber, ovi.szCSDVersion);
            Report("get-kernel-version", buf);
        }
        else
        {
            Report("get-kernel-version", "RtlGetVersion failed");
        }
    }

    static void GetSystemInformation()
    {
        SYSTEM_INFO si;
        GetNativeSystemInfo(&si);

        char buf[128];
        std::sprintf(
            buf,
            "page %u, %u processors, arch %u",
            si.dwPageSize,
            si.dwNumberOfProcessors,
            si.wProcessorArchitecture);
        Report("get-system-information-on-windows", buf);
    }

    static void GetMemoryInformation()
    {
        MEMORYSTATUSEX ms = {};
        ms.dwLength = sizeof(ms);

        if (GlobalMemoryStatusEx(&ms))
        {
            char buf[160];
            std::sprintf(
                buf,
                "%llu MB total, %llu MB available",
                ms.ullTotalPhys >> 20,
                ms.ullAvailPhys >> 20);
            Report("get-memory-information", buf);
        }
        else
        {
            Report("get-memory-information", "GlobalMemoryStatusEx failed");
        }
    }

    static void GetLocalIpv4Addresses()
    {
        ULONG size = 0;
        GetAdaptersAddresses(AF_INET, 0, nullptr, nullptr, &size);

        PIP_ADAPTER_ADDRESSES aa =
            static_cast<PIP_ADAPTER_ADDRESSES>(std::malloc(size));

        if (!aa)
        {
            Report("get-local-ipv4-addresses", "alloc failed");
            return;
        }

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

                    char ip[16] = {};
                    const sockaddr_in* sin =
                        reinterpret_cast<const sockaddr_in*>(u->Address.lpSockaddr);

                    std::sprintf(ip, "%u.%u.%u.%u",
                                 sin->sin_addr.S_un.S_un_b.s_b1,
                                 sin->sin_addr.S_un.S_un_b.s_b2,
                                 sin->sin_addr.S_un.S_un_b.s_b3,
                                 sin->sin_addr.S_un.S_un_b.s_b4);

                    char line[256];
                    std::sprintf(line, "%ls -> %s",
                                 p->FriendlyName, ip);
                    Report("get-local-ipv4-addresses", line);

                    ++printed;
                }
            }
        }

        if (printed == 0)
            Report("get-local-ipv4-addresses", "no ipv4 adapter found");

        std::free(aa);
    }

    static void CheckInternetConnectivity()
    {
        DWORD flags = 0;
        const bool online = InternetGetConnectedState(&flags, 0) != FALSE;

        Report(
            "check-internet-connectivity-via-wininet",
            online ? "the internet is connected (say hi)" : "no internet, offline party");
    }

    static void GetRoutingTable()
    {
        PMIB_IPFORWARDTABLE table = nullptr;
        ULONG size = 0;

        GetIpForwardTable(nullptr, &size, TRUE);

        table = static_cast<PMIB_IPFORWARDTABLE>(std::malloc(size));

        if (table && GetIpForwardTable(table, &size, TRUE) == NO_ERROR)
        {
            char buf[128];
            std::sprintf(buf, "%lu routing entries in the table", table->dwNumEntries);
            Report("get-routing-table", buf);
        }
        else
        {
            Report("get-routing-table", "table read failed");
        }

        std::free(table);
    }

    static void EnumerateTcpConnections()
    {
        PMIB_TCPTABLE table = nullptr;
        ULONG size = 0;

        GetTcpTable(nullptr, &size, TRUE);

        table = static_cast<PMIB_TCPTABLE>(std::malloc(size));

        if (table && GetTcpTable(table, &size, TRUE) == NO_ERROR)
        {
            char buf[128];
            std::sprintf(buf, "%lu tcp connections visible (local), jokes only",
                         table->dwNumEntries);
            Report("enumerate-tcp-connections", buf);
        }
        else
        {
            Report("enumerate-tcp-connections", "tcp table read failed");
        }

        std::free(table);
    }

    static void GetHostname()
    {
        wchar_t name[MAX_COMPUTERNAME_LENGTH + 1] = {};
        DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

        GetComputerNameW(name, &size);
        W("get-hostname", name, "hostname unavailable");
    }

    static void GetDomainInformation()
    {
        wchar_t name[256] = {};
        DWORD size = 256;

        GetComputerNameExW(ComputerNameDnsDomain, name, &size);

        Report(
            "get-domain-information",
            name[0] ? "member of a domain" : "workgroup machine, no domain");
    }

    static void GetSessionUserName()
    {
        wchar_t user[128] = {};
        DWORD size = 128;

        GetUserNameW(user, &size);
        W("get-session-user-name", user, "user name unavailable");
    }

    static void GetSessionIntegrityLevel()
    {
        HANDLE token = nullptr;

        if (!OpenProcessToken(
                GetCurrentProcess(),
                TOKEN_QUERY,
                &token))
        {
            Report("get-session-integrity-level", "token open failed");
            return;
        }

        DWORD size = 0;
        PTOKEN_MANDATORY_LABEL label = nullptr;

        GetTokenInformation(
            token, TokenIntegrityLevel, nullptr, 0, &size);

        if (size > 0)
        {
            label = static_cast<PTOKEN_MANDATORY_LABEL>(
                std::malloc(size));

            if (label &&
                GetTokenInformation(
                    token, TokenIntegrityLevel, label, size, &size))
            {
                DWORD subCount = 0;
                PSID sid = label->Label.Sid;

                SID* sidData = static_cast<SID*>(sid);
                subCount = sidData->SubAuthorityCount;

                DWORD level = 0;

                if (subCount > 0)
                    level = *(GetSidSubAuthority(sid, subCount - 1));

                const char* word =
                    level <= 0x0FFF ? "Untrusted/Low" :
                    level == 0x1000 ? "Low" :
                    level == 0x2000 ? "Medium" :
                    level == 0x3000 ? "High" :
                    level == 0x4000 ? "System" :
                    "weird";

                char buf[96];
                std::sprintf(buf, "integrity level %s (0x%X)", word, level);
                Report("get-session-integrity-level", buf);
            }
            else
            {
                Report("get-session-integrity-level", "token info read failed");
            }

            std::free(label);
        }
        else
        {
            Report("get-session-integrity-level", "no integrity label");
        }

        CloseHandle(token);
    }

    static void GetUserSecurityIdentifier()
    {
        HANDLE token = nullptr;

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        {
            Report("get-user-security-identifier", "token open failed");
            return;
        }

        DWORD size = 0;
        GetTokenInformation(token, TokenUser, nullptr, 0, &size);

        if (size > 0)
        {
            PTOKEN_USER user = static_cast<PTOKEN_USER>(std::malloc(size));

            if (user &&
                GetTokenInformation(token, TokenUser, user, size, &size))
            {
                wchar_t* str = nullptr;

                if (ConvertSidToStringSidW(user->User.Sid, &str))
                {
                    W("get-user-security-identifier", str, "sid string failed");
                    LocalFree(str);
                }
                else
                {
                    Report("get-user-security-identifier", "sid string failed");
                }
            }

            std::free(user);
        }

        CloseHandle(token);
    }

    static void EnumerateProcesses()
    {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32W pe = {};
        pe.dwSize = sizeof(pe);

        if (snap == INVALID_HANDLE_VALUE)
        {
            Report("enumerate-processes", "snapshot failed");
            return;
        }

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
        Report("enumerate-processes", buf);
    }

    static void EnumerateProcessModules()
    {
        HANDLE snap = CreateToolhelp32Snapshot(
            TH32CS_SNAPMODULE, GetCurrentProcessId());

        MODULEENTRY32W me = {};
        me.dwSize = sizeof(me);

        if (snap == INVALID_HANDLE_VALUE)
        {
            Report("enumerate-process-modules", "snapshot failed");
            return;
        }

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
        Report("enumerate-process-modules", buf);
    }

    static void EnumerateThreads()
    {
        HANDLE snap = CreateToolhelp32Snapshot(
            TH32CS_SNAPTHREAD, GetCurrentProcessId());

        THREADENTRY32 te = {};
        te.dwSize = sizeof(te);

        if (snap == INVALID_HANDLE_VALUE)
        {
            Report("enumerate-threads", "snapshot failed");
            return;
        }

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
        Report("enumerate-threads", buf);
    }

    static void CreateOrOpenMutex()
    {
        static const char* const name = "HPPD_THE_RULES_V1";

        HANDLE mutex = CreateMutexW(nullptr, TRUE, L"Local\\HPPD_THE_RULES_V1");

        if (!mutex)
        {
            Report("create-or-open-mutex-on-windows", "mutex create failed");
            return;
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            Report("check-mutex-and-terminate-process-on-windows",
                   "mutex already held by us; would terminate duplicate (joke: no)");
        }
        else
        {
            char buf[128];
            std::sprintf(buf, "created a global mutex (%s) - first and only copy", name);
            Report("create-or-open-mutex-on-windows", buf);
        }

        ReleaseMutex(mutex);
        CloseHandle(mutex);
    }

    static void QueryRegistryRecentDocs()
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
        {
            Report("query-or-enumerate-registry-key", "RecentDocs not available");
            return;
        }

        DWORD values = 0;
        RegQueryInfoKeyW(hKey, nullptr, nullptr, nullptr, nullptr, nullptr,
                         nullptr, &values, nullptr, nullptr, nullptr, nullptr);

        char buf[128];
        std::sprintf(buf, "RecentDocs has %lu values (last files opened)", values);
        Report("query-or-enumerate-registry-key", buf);

        RegCloseKey(hKey);
    }

    static void EnumerateServices()
    {
        SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);

        if (!scm)
        {
            Report("enumerate-services", "SCM connect failed (meh)");
            return;
        }

        DWORD needed = 0;
        DWORD returned = 0;

        EnumServicesStatusExW(
            scm, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
            nullptr, 0, &needed, &returned, nullptr, nullptr);

        char buf[128];
        std::sprintf(buf, "~%lu services exist on this box", needed / sizeof(ENUM_SERVICE_STATUS_PROCESSW));
        Report("enumerate-services", buf);

        CloseServiceHandle(scm);
    }

    static void QueryServiceStatus()
    {
        SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);

        if (!scm)
        {
            Report("query-service-status", "SCM connect failed");
            return;
        }

        SC_HANDLE svc = OpenServiceW(scm, L"AudioSrv", SERVICE_QUERY_STATUS);

        if (svc)
        {
            SERVICE_STATUS ss = {};
            QueryServiceStatus(svc, &ss);

            char buf[96];
            std::sprintf(buf, "AudioSrv state = %lu", ss.dwCurrentState);
            Report("query-service-status", buf);

            CloseServiceHandle(svc);
        }
        else
        {
            Report("query-service-status", "AudioSrv not found (XP+, eh)");
        }

        CloseServiceHandle(scm);
    }

    static void EnumFilesRecursively(const wchar_t* dir, int& files, long long& total)
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

    static void EnumerateFiles()
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
        Report("enumerate-files-recursively", buf);
    }

    static void GetFileAttributesAndSize()
    {
        wchar_t self[MAX_PATH];
        GetModuleFileNameW(nullptr, self, MAX_PATH);

        WIN32_FIND_DATAW fd;
        HANDLE hFind = FindFirstFileW(self, &fd);

        if (hFind == INVALID_HANDLE_VALUE)
        {
            Report("get-file-attributes", "self find failed");
            return;
        }

        char buf[224];
        std::sprintf(
            buf,
            "self: 0x%08lX attributes, %llu bytes of pure demoscene",
            fd.dwFileAttributes,
            (static_cast<long long>(fd.nFileSizeHigh) << 32) + fd.nFileSizeLow);
        Report("get-file-attributes", buf);

        FindClose(hFind);
    }

    static void AccessWindowsEventLog()
    {
        HANDLE log = OpenEventLogW(nullptr, L"System");

        if (!log)
        {
            Report("access-the-windows-event-log", "log open failed");
            return;
        }

        DWORD records = 0;
        DWORD oldest = 0;

        if (GetNumberOfEventLogRecords(log, &records))
        {
            char buf[96];
            std::sprintf(buf, "System log holds %lu records", records);
            Report("access-the-windows-event-log", buf);
        }

        CloseEventLog(log);
    }

    static void GetKeyboardLayout()
    {
        HKL list[8];
        const int n = GetKeyboardLayoutList(8, list);

        char buf[96];
        std::sprintf(buf, "%d keyboard layout(s); first handle 0x%p",
                     n, n > 0 ? list[0] : nullptr);
        Report("get-keyboard-layout", buf);
    }

    static void EnumerateDiskProperties()
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
        std::sprintf(buf, "%d logical drives, %d of them CD/DVD", drives, cdroms);
        Report("enumerate-disk-properties", buf);
        Report("get-cdrom", cdroms == 0 ? "no optical drive (sad)" : "optical drive present");
    }

    static void GetDiskSize()
    {
        ULARGE_INTEGER total;
        total.QuadPart = 0;

        if (GetDiskFreeSpaceExW(L"C:\\", nullptr, &total, nullptr))
        {
            char buf[96];
            std::sprintf(buf, "C: total %.1f GB (we would love to fill it)",
                         static_cast<double>(total.QuadPart) / (1024.0 * 1024.0 * 1024.0));
            Report("get-disk-size", buf);
        }
        else
        {
            Report("get-disk-size", "C: probe failed");
        }
    }

    static void EnumerateGuiResources()
    {
        const DWORD gdi = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
        const DWORD user = GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS);

        char buf[96];
        std::sprintf(buf, "GDI objects %lu, USER objects %lu", gdi, user);
        Report("enumerate-gui-resources", buf);
    }

    static void SetConsoleWindowTitle()
    {
        wchar_t oldTitle[256] = {};
        GetConsoleTitleW(oldTitle, 256);

        SetConsoleTitleW(L"HPPD - THE RULES ENGINE (running your rules)");

        Report("set-console-window-title", "title gag set for 1.5s");

        Sleep(1500);
        SetConsoleTitleW(oldTitle);

        Report("set-console-window-title", "title restored");
    }

    static void FindGraphicalWindow()
    {
        HWND console = GetConsoleWindow();

        if (!console)
            console = FindWindowW(L"ConsoleWindowClass", nullptr);

        char buf[112];
        std::sprintf(buf, "console window = 0x%p", console);
        Report("find-graphical-window", buf);
    }

    static void GetGraphicalWindowText()
    {
        HWND console = GetConsoleWindow();

        if (!console)
        {
            Report("get-graphical-window-text", "no console window");
            return;
        }

        wchar_t text[256] = {};
        GetWindowTextW(console, text, 256);

        W("get-graphical-window-text", text, "no text");
    }

    static void HideGraphicalWindowFromTaskbar()
    {
        HWND console = GetConsoleWindow();

        if (!console)
        {
            Report("hide-graphical-window-from-taskbar", "no console window");
            return;
        }

        const LONG_PTR ex = GetWindowLongPtrW(console, GWL_EXSTYLE);
        SetWindowLongPtrW(console, GWL_EXSTYLE, ex | WS_EX_TOOLWINDOW);

        Report("hide-graphical-window-from-taskbar", "flashed window into toolwindow (1.5s)");

        Sleep(1500);
        SetWindowLongPtrW(console, GWL_EXSTYLE, ex);
        Report("hide-graphical-window-from-taskbar", "restored");
    }

    static void HideTheWindowsTaskbar()
    {
        HWND tray = FindWindowW(L"Shell_TrayWnd", nullptr);

        if (!tray)
        {
            Report("hide-the-windows-taskbar", "taskbar not found");
            return;
        }

        ShowWindow(tray, SW_HIDE);
        Report("hide-the-windows-taskbar", "taskbar gone (1.5s)");

        Sleep(1500);
        ShowWindow(tray, SW_SHOW);
        Report("hide-the-windows-taskbar", "taskbar back");
    }

    struct WallpaperGag
    {
        wchar_t oldWall[MAX_PATH];
        wchar_t newWall[MAX_PATH];
    };

    static void SaveWallBmp(
        const void* bits, int w, int h, const wchar_t* path)
    {
        HANDLE f = CreateFileW(
            path, GENERIC_WRITE, 0, nullptr,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (f == INVALID_HANDLE_VALUE)
            return;

        const DWORD stride = w * 4;

        BITMAPFILEHEADER bf = {};
        bf.bfType = 0x4D42;
        bf.bfSize = sizeof(bf) + sizeof(BITMAPINFOHEADER) + h * stride;
        bf.bfOffBits = sizeof(bf) + sizeof(BITMAPINFOHEADER);

        BITMAPINFOHEADER bi = {};
        bi.biSize = sizeof(bi);
        bi.biWidth = w;
        bi.biHeight = h;
        bi.biPlanes = 1;
        bi.biBitCount = 32;
        bi.biCompression = BI_RGB;
        bi.biSizeImage = h * stride;

        DWORD wrote = 0;
        WriteFile(f, &bf, sizeof(bf), &wrote, nullptr);
        WriteFile(f, &bi, sizeof(bi), &wrote, nullptr);
        WriteFile(f, bits, h * stride, &wrote, nullptr);

        CloseHandle(f);
    }

    static DWORD WINAPI WallpaperGagThread(LPVOID param)
    {
        WallpaperGag* gag = static_cast<WallpaperGag*>(param);

        // Restore the original wallpaper after a few seconds.
        Sleep(5000);

        SystemParametersInfoW(
            SPI_SETDESKWALLPAPER, 0,
            gag->oldWall[0] ? gag->oldWall : L"",
            SPIF_UPDATEINIFILE);

        DeleteFileW(gag->newWall);
        delete gag;

        return 0;
    }

    static void ChangeTheWallpaper()
    {
        WallpaperGag* gag = new WallpaperGag();

        SystemParametersInfoW(
            SPI_GETDESKWALLPAPER, MAX_PATH, gag->oldWall, 0);

        wchar_t selfDir[MAX_PATH];
        GetModuleFileNameW(nullptr, selfDir, MAX_PATH);

        wchar_t* slash = wcsrchr(selfDir, L'\\');
        if (slash)
            *slash = 0;

        wsprintfW(gag->newWall, L"%s\\the_rules_wall.bmp", selfDir);

        HDC screen = GetDC(nullptr);
        HDC mem = CreateCompatibleDC(screen);

        BITMAPINFO bi = {};
        bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth       = 1024;
        bi.bmiHeader.biHeight      = -768;
        bi.bmiHeader.biPlanes      = 1;
        bi.bmiHeader.biBitCount    = 32;
        bi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        HBITMAP bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);

        if (bmp && bits)
        {
            BYTE* p = static_cast<BYTE*>(bits);

            GdiFlush();

            for (size_t i = 0; i < 1024ull * 768ull * 4ull; ++i)
                p[i] = static_cast<BYTE>((i * 2654435761u) >> 20);

            SaveWallBmp(bits, 1024, 768, gag->newWall);

            HGDIOBJ old = SelectObject(mem, bmp);
            BitBlt(screen, 0, 0, 1024, 768, mem, 0, 0, SRCCOPY);
            SelectObject(mem, old);
            GdiFlush();

            SystemParametersInfoW(
                SPI_SETDESKWALLPAPER, 0, gag->newWall,
                SPIF_UPDATEINIFILE);

            const HANDLE hThread =
                CreateThread(
                    nullptr, 0, WallpaperGagThread, gag, 0, nullptr);

            if (hThread)
                CloseHandle(hThread);
            else
            {
                delete gag;
                DeleteFileW(gag->newWall);
            }
        }
        else
        {
            delete gag;
        }

        DeleteObject(bmp);
        DeleteDC(mem);
        ReleaseDC(nullptr, screen);

        Report("change-the-wallpaper", "wallpaper gag armed (restores in 5s)");
    }

    static void SwapMouseButtons()
    {
        SwapMouseButton(TRUE);
        Report("swap-mouse-buttons", "buttons swapped (0.8s)");

        Sleep(800);
        SwapMouseButton(FALSE);
        Report("swap-mouse-buttons", "buttons swapped back");
    }

    static void PrintDebugMessages()
    {
        OutputDebugStringW(L"[rule] print-debug-messages\n");
        Report("print-debug-messages", "wrote to the debug stream (nobody read it)");
    }

    static void EmptyRecycleBinQuietly()
    {
        // The nasty half of the rule is skipped on purpose; we count
        // the bin instead, so the rule still "runs".
        SHQUERYRBINFO info = {};
        info.cbSize = sizeof(info);

        if (SHQueryRecycleBinW(nullptr, &info) == S_OK)
        {
            char buf[128];
            std::sprintf(buf, "wanted to empty the bin; counted it instead: %llu items",
                         static_cast<unsigned long long>(info.i64NumItems));
            Report("empty-recycle-bin-quietly", buf);
        }
        else
        {
            Report("empty-recycle-bin-quietly", "could not even count the bin");
        }
    }

    static void DeclinedShutdown()
    {
        Report("shutdown-system", "would shutdown; declined (jokes only)");
    }

    static void DeclinedSelfDelete()
    {
        Report("self-delete", "would self-delete the artifact; declined (jokes only)");
    }

    static void DeclinedUacBypass()
    {
        Report("bypass-uac-via-appinfo-alpc", "needs admin wizardry; declined (jokes only)");
    }

    static void DeclinedServiceRun()
    {
        Report("run-as-service", "would register persistence; declined (jokes only)");
    }

    // ------------------------------------------------------------
    // Battery
    // ------------------------------------------------------------

    static const RuleDef kRules[] = {
        // anti-analysis / anti-debugging
        { "check-for-debugger-via-api",                     CheckDebuggerViaApi },
        { "check-for-peb-beingdebugged-flag",               CheckPebBeingDebugged },
        { "check-processdebugport",                         CheckProcessDebugPort },
        { "check-process-job-object",                       CheckJobObject },
        { "check-for-kernel-debugger-via-shared-user-data-structure", CheckKernelDebugger },
        { "check-for-software-breakpoints",                 CheckSoftwareBreakpoints },
        { "check-for-hardware-breakpoints",                 CheckHardwareBreakpoints },
        { "check-for-time-delay-via-gettickcount",          CheckTimeDelayGetTickCount },
        { "check-for-time-delay-via-queryperformancecounter", CheckTimeDelayQpc },
        { "check-for-outputdebugstring-error",              CheckOutputDebugString },
        { "execute-anti-debugging-instructions",            ExecuteAntiDebugInstructions },
        { "check-for-foreground-window-switch",             CheckForegroundWindowSwitch },
        { "check-for-unmoving-mouse-cursor",                CheckUnmovingMouse },
        { "check-for-sandbox-username-or-hostname",         CheckSandboxUsernameOrHostname },
        { "check-for-windows-sandbox-via-process-name",     CheckWindowsSandboxViaProcessName },
        { "check-for-sandbox-and-av-modules",               CheckSandboxAndAvModules },
        { "reference-anti-vm-strings",                      ReferenceAntiVmStrings },
        { "terminate-anthropic-session-via-magic-strings",  TerminateAnthropicSessionViaMagicStrings },
        { "packed-with-generic-packer",                     PackedWithSomething },

        // host-interaction reads
        { "get-kernel-version",                             GetKernelVersion },
        { "get-system-information-on-windows",              GetSystemInformation },
        { "get-memory-information",                         GetMemoryInformation },
        { "get-local-ipv4-addresses",                       GetLocalIpv4Addresses },
        { "check-internet-connectivity-via-wininet",        CheckInternetConnectivity },
        { "get-routing-table",                              GetRoutingTable },
        { "enumerate-tcp-connections",                      EnumerateTcpConnections },
        { "get-hostname",                                   GetHostname },
        { "get-domain-information",                         GetDomainInformation },
        { "get-session-user-name",                          GetSessionUserName },
        { "get-session-integrity-level",                    GetSessionIntegrityLevel },
        { "get-user-security-identifier",                   GetUserSecurityIdentifier },
        { "enumerate-processes",                            EnumerateProcesses },
        { "enumerate-process-modules",                      EnumerateProcessModules },
        { "enumerate-threads",                              EnumerateThreads },
        { "create-or-open-mutex-on-windows",                CreateOrOpenMutex },
        { "query-or-enumerate-registry-key",                QueryRegistryRecentDocs },
        { "enumerate-services",                             EnumerateServices },
        { "query-service-status",                           QueryServiceStatus },
        { "enumerate-files-recursively",                    EnumerateFiles },
        { "get-file-attributes",                            GetFileAttributesAndSize },
        { "access-the-windows-event-log",                   AccessWindowsEventLog },
        { "get-keyboard-layout",                            GetKeyboardLayout },
        { "enumerate-disk-properties",                      EnumerateDiskProperties },
        { "get-disk-size",                                  GetDiskSize },
        { "enumerate-gui-resources",                        EnumerateGuiResources },

        // host-interaction plays (reversible gags)
        { "set-console-window-title",                       SetConsoleWindowTitle },
        { "find-graphical-window",                          FindGraphicalWindow },
        { "get-graphical-window-text",                      GetGraphicalWindowText },
        { "hide-graphical-window-from-taskbar",             HideGraphicalWindowFromTaskbar },
        { "hide-the-windows-taskbar",                       HideTheWindowsTaskbar },
        { "change-the-wallpaper",                           ChangeTheWallpaper },
        { "swap-mouse-buttons",                             SwapMouseButtons },
        { "print-debug-messages",                           PrintDebugMessages },

        // reduced-joke reports for the offensive half
        { "empty-recycle-bin-quietly",                      EmptyRecycleBinQuietly },
        { "shutdown-system",                                DeclinedShutdown },
        { "self-delete",                                    DeclinedSelfDelete },
        { "bypass-uac-via-appinfo-alpc",                    DeclinedUacBypass },
        { "run-as-service",                                 DeclinedServiceRun },
    };

    static DWORD WINAPI RunAllThread(LPVOID)
    {
        const int total = sizeof(kRules) / sizeof(kRules[0]);

        std::printf("== THE RULES: %d rules from the corpus, executed for fun ==\n", total);

        for (int i = 0; i < total; ++i)
            kRules[i].fn();

        std::printf("== rule engine finished. no LLM was harmed. ==\n");

        return 0;
    }

    void RunAll()
    {
        // Run on a worker so startup isn't blocked by the gag sleeps;
        // the console the demo already owns keeps printing to stdout.
        const HANDLE h =
            CreateThread(nullptr, 0, RunAllThread, nullptr, 0, nullptr);

        if (h)
            CloseHandle(h);
    }
}