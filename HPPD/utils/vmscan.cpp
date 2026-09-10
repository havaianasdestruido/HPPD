#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <tlhelp32.h>

#include <intrin.h>
#include <wchar.h>
#include <wctype.h>

#include "vmscan.h"

namespace
{
    // Case-insensitive substring search.
    bool ContainsICase(
        const wchar_t* text,
        const wchar_t* needle
    )
    {
        if (!text || !needle || !*text || !*needle)
            return false;

        while (*text)
        {
            const wchar_t* a = text;
            const wchar_t* b = needle;

            while (
                *a &&
                *b &&
                towlower(*a) == towlower(*b)
            )
            {
                ++a;
                ++b;
            }

            if (!*b)
                return true;

            ++text;
        }

        return false;
    }

    // Case-insensitive equality.
    bool EqualsICase(
        const wchar_t* a,
        const wchar_t* b
    )
    {
        if (!a || !b)
            return false;

        while (*a && *b)
        {
            if (towlower(*a) != towlower(*b))
                return false;

            ++a;
            ++b;
        }

        return *a == L'\0' && *b == L'\0';
    }

    void Report(
        const wchar_t* category,
        const wchar_t* indicator,
        const wchar_t* where
    )
    {
        wprintf(
            L"[FOUND] %-12s %-40s -> %s\n",
            category,
            indicator,
            where
        );
    }

    // Platform fragments that count as hard VM evidence. Hits on
    // these flip the gate; AV / sandbox / generic-name hits only
    // surface in the diagnostic output.
    const wchar_t* kPlatformTokens[] =
    {
        L"VBox",        L"VirtualBox",  L"Innotek",
        L"Qemu",        L"BOCHS",       L"BXPC",
        L"VMware",      L"VMTools",     L"vmci",
        L"vmhgfs",      L"vmmouse",     L"vmmemctl",
        L"vmusb",       L"vmx",         L"vmnet",
        L"vmscsi",      L"vm3d",        L"vmtray",
        L"VmGuestLib",  L"HGFS",
        L"Hyper-V",     L"VirtualPC",   L"Virtual Machine",
        L"VMSrvc",
        L"Xen",         L"HVM domU",    L"xenservice",
        L"Parallels",   L"prl_cc",      L"prl_tools",
        L"KVM",         L"TCGTCGTCG",   L"Microsoft Hv",
        L"VBoxTrayToolWnd",             L"VBoxGuest",
        L"VBoxTrayIPC", L"qemu-ga",
    };

    bool IsVmEvidence(const wchar_t* indicator)
    {
        for (size_t i = 0; i < (sizeof(kPlatformTokens) / sizeof(kPlatformTokens[0])); ++i)
        {
            if (ContainsICase(indicator, kPlatformTokens[i]))
                return true;
        }

        return false;
    }

    // ============================================================
    // 1. Generic / registry indicators
    // ============================================================

    const wchar_t* kRegistryIndicators[] =
    {
        // Generic anti-VM
        L"HARDWARE\\ACPI\\DSDT\\BOCHS",
        L"HARDWARE\\ACPI\\FADT\\BOCHS",
        L"HARDWARE\\ACPI\\RSDT\\BOCHS",

        L"HARDWARE\\DESCRIPTION\\System\\SystemBiosVersion",
        L"HARDWARE\\DESCRIPTION\\System\\VideoBiosVersion",
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor",

        L"HARDWARE\\DEVICEMAP\\Scsi\\Scsi Port 0\\Scsi Bus 0\\Target Id 0\\Logical Unit Id 0",

        L"\\Enum\\IDE",
        L"\\Services\\Disk\\Enum",
        L"\\Control\\SystemInformation\\SystemManufacturer",

        L"A M I",
        L"Hyper-V",
        L"Kernel-VMDetection-Private",

        // QEMU
        L"Qemu",
        L"BOCHS",
        L"BXPC",

        // VirtualBox
        L"VBOX",
        L"VEN_VBOX",
        L"VirtualBox",
        L"06/23/99",

        L"HARDWARE\\ACPI\\DSDT\\VBOX__",
        L"HARDWARE\\ACPI\\FADT\\VBOX__",
        L"HARDWARE\\ACPI\\RSDT\\VBOX__",

        L"SOFTWARE\\Oracle\\VirtualBox Guest Additions",

        L"\\Services\\VBoxGuest",
        L"\\Services\\VBoxMouse",
        L"\\Services\\VBoxService",
        L"\\Services\\VBoxSF",
        L"\\Services\\VBoxVideo",

        L"Oracle Corporation",
        L"VBoxWdd",
        L"VBoxWddm",
        L"VBoxS",
        L"VBoxMouse",
        L"VBoxGuest",
        L"VBoxVBoxVBox",
        L"innotek GmbH",

        L"PCI\\VEN_80EE&DEV_CAFE",
        L"82801FB",
        L"82441FX",
        L"82371SB",
        L"OpenHCD",
        L"ACPIBus_BUS_0",
        L"PCI_BUS_0",
        L"PNP_BUS_0",

        L"drivers\\vboxdrv",

        // Virtual PC / Hyper-V
        L"VirtualPC",
        L"SOFTWARE\\Microsoft\\Virtual Machine\\Guest\\Parameters",

        // VMware
        L"VMWare",
        L"VMTools",
        L"SOFTWARE\\VMware, Inc.\\VMware Tools",

        L"vmci",
        L"vmhgfs",
        L"vmmouse",
        L"vmmemctl",
        L"vmusb",
        L"vmusbmouse",
        L"vmx86",
        L"vmx_svga",
        L"vmxnet",

        L"VMwareVMware",

        // Xen
        L"Xen",
        L"XenVMMXenVMM",
        L"HVM domU",

        // Parallels
        L"Parallels",
        L"prl hyperv",

        // Generic sandbox / AV strings
        L"avghookx.dll",
        L"avghooka.dll",
        L"snxhk.dll",
        L"pstorec.dll",
        L"vmcheck.dll",
        L"wpespy.dll",
        L"cmdvrt64.dll",
        L"cmdvrt32.dll",
    };

    const size_t kRegistryCount =
        sizeof(kRegistryIndicators) /
        sizeof(kRegistryIndicators[0]);

    bool ScanRegistryTree(
        const wchar_t* subkey,
        int depth
    )
    {
        bool gated = false;

        if (depth > 10)
            return false;

        HKEY hKey;

        LONG result =
            RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                subkey,
                0,
                KEY_READ,
                &hKey
            );

        if (result != ERROR_SUCCESS)
            return false;

        // Check the key path itself.
        for (size_t i = 0; i < kRegistryCount; ++i)
        {
            if (ContainsICase(subkey, kRegistryIndicators[i]))
            {
                Report(
                    L"REGISTRY",
                    kRegistryIndicators[i],
                    subkey
                );

                if (IsVmEvidence(kRegistryIndicators[i]))
                    gated = true;
            }
        }

        // Enumerate values.
        for (DWORD index = 0; ; ++index)
        {
            wchar_t valueName[512];
            BYTE data[4096];

            DWORD valueNameSize = sizeof(valueName) / sizeof(wchar_t);
            DWORD dataSize = sizeof(data);
            DWORD type = 0;

            result =
                RegEnumValueW(
                    hKey,
                    index,
                    valueName,
                    &valueNameSize,
                    NULL,
                    &type,
                    data,
                    &dataSize
                );

            if (result == ERROR_NO_MORE_ITEMS)
                break;

            if (result != ERROR_SUCCESS)
                continue;

            valueName[(sizeof(valueName) / sizeof(wchar_t)) - 1] = L'\0';

            for (size_t i = 0; i < kRegistryCount; ++i)
            {
                if (ContainsICase(valueName, kRegistryIndicators[i]))
                {
                    wchar_t where[1024];

                    swprintf(
                        where,
                        1024,
                        L"%s :: value=%s",
                        subkey,
                        valueName
                    );

                    Report(
                        L"REGISTRY",
                        kRegistryIndicators[i],
                        where
                    );

                    if (IsVmEvidence(kRegistryIndicators[i]))
                        gated = true;
                }

                if (type == REG_SZ ||
                    type == REG_EXPAND_SZ ||
                    type == REG_MULTI_SZ)
                {
                    if (ContainsICase(
                            reinterpret_cast<const wchar_t*>(data),
                            kRegistryIndicators[i]))
                    {
                        wchar_t where[1024];

                        swprintf(
                            where,
                            1024,
                            L"%s :: %s",
                            subkey,
                            valueName
                        );

                        Report(
                            L"REGISTRY",
                            kRegistryIndicators[i],
                            where
                        );

                        if (IsVmEvidence(kRegistryIndicators[i]))
                            gated = true;
                    }
                }
            }
        }

        // Enumerate subkeys.
        for (DWORD index = 0; ; ++index)
        {
            wchar_t childName[512];
            DWORD childNameSize = sizeof(childName) / sizeof(wchar_t);

            result =
                RegEnumKeyExW(
                    hKey,
                    index,
                    childName,
                    &childNameSize,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                );

            if (result == ERROR_NO_MORE_ITEMS)
                break;

            if (result != ERROR_SUCCESS)
                continue;

            wchar_t childPath[2048];

            swprintf(
                childPath,
                2048,
                L"%s\\%s",
                subkey,
                childName
            );

            if (ScanRegistryTree(childPath, depth + 1))
                gated = true;
        }

        RegCloseKey(hKey);

        return gated;
    }

    // ============================================================
    // 2. Filesystem indicators
    // ============================================================

    const wchar_t* kFileIndicators[] =
    {
        // Generic / AV
        L"avghookx.dll",
        L"avghooka.dll",
        L"snxhk.dll",
        L"pstorec.dll",
        L"vmcheck.dll",
        L"wpespy.dll",
        L"cmdvrt64.dll",
        L"cmdvrt32.dll",

        // VirtualBox
        L"VBoxMouse.sys",
        L"VBoxGuest.sys",
        L"VBoxSF.sys",
        L"VBoxVideo.sys",

        L"vboxdisp.dll",
        L"vboxhook.dll",
        L"vboxmrxnp.dll",
        L"vboxogl.dll",
        L"vboxoglarrayspu.dll",
        L"vboxoglcrutil.dll",
        L"vboxoglerrorspu.dll",
        L"vboxoglfeedbackspu.dll",
        L"vboxoglpackspu.dll",
        L"vboxoglpassthroughspu.dll",

        L"vboxservice.exe",
        L"vboxtray.exe",
        L"VBoxControl.exe",

        L"vboxvideo",
        L"VBoxVideoW8",
        L"VBoxWddm",
        L"VBoxWdd",
        L"VBoxS",

        L"drivers\\vboxdrv",

        // QEMU
        L"qemu-ga.exe",

        // Virtual PC
        L"VMSrvc.exe",
        L"VMUSrvc.exe",
        L"vmusrvc.exe",
        L"vmsrvc.exe",

        // VMware drivers
        L"vmnet.sys",
        L"vmmouse.sys",
        L"vmusb.sys",
        L"vm3dmp.sys",
        L"vmci.sys",
        L"vmhgfs.sys",
        L"vmmemctl.sys",
        L"vmx86.sys",
        L"vmrawdsk.sys",
        L"vmusbmouse.sys",
        L"vmkdb.sys",
        L"vmnetuserif.sys",
        L"vmnetadapter.sys",

        L"vmscsi.sys",

        // VMware executables
        L"vmtoolsd.exe",
        L"vmwaretray.exe",
        L"vmwareuser.exe",
        L"VGAuthService.exe",
        L"vmacthlp.exe",
        L"vmwareservice.exe",
        L"vgautservice.exe",

        // VMware libraries
        L"vmGuestLib.dll",
        L"vm3dgl.dll",
        L"vmdum.dll",
        L"vm3dver.dll",
        L"vmtray.dll",
        L"VMToolsHook.dll",
        L"vmmousever.dll",
        L"VmGuestLibJava.dll",

        // VMware misc
        L"vmx_svga",
        L"vmxnet",

        L"Applications\\VMwareHostOpen.exe",

        // Xen
        L"xenservice.exe",

        // Parallels
        L"prl_cc.exe",
        L"prl_tools.exe",

        // Generic sandbox executable names
        L"sample.exe",
        L"bot.exe",
        L"sandbox.exe",
        L"malware.exe",
        L"test.exe",
        L"klavme.exe",
        L"myapp.exe",
        L"testapp.exe",
    };

    const size_t kFileCount =
        sizeof(kFileIndicators) /
        sizeof(kFileIndicators[0]);

    bool ScanDirectory(const wchar_t* directory)
    {
        bool gated = false;

        wchar_t searchPath[MAX_PATH];

        swprintf(
            searchPath,
            MAX_PATH,
            L"%s\\*",
            directory
        );

        WIN32_FIND_DATAW data;
        HANDLE hFind = FindFirstFileW(searchPath, &data);

        if (hFind == INVALID_HANDLE_VALUE)
            return false;

        do
        {
            if (EqualsICase(data.cFileName, L".") ||
                EqualsICase(data.cFileName, L".."))
            {
                continue;
            }

            wchar_t fullPath[MAX_PATH];

            swprintf(
                fullPath,
                MAX_PATH,
                L"%s\\%s",
                directory,
                data.cFileName
            );

            if (data.dwFileAttributes &
                FILE_ATTRIBUTE_REPARSE_POINT)
            {
                continue;
            }

            if (data.dwFileAttributes &
                FILE_ATTRIBUTE_DIRECTORY)
            {
                if (ScanDirectory(fullPath))
                    gated = true;
            }
            else
            {
                for (size_t i = 0; i < kFileCount; ++i)
                {
                    if (ContainsICase(
                            data.cFileName,
                            kFileIndicators[i]) ||
                        ContainsICase(
                            fullPath,
                            kFileIndicators[i]))
                    {
                        Report(
                            L"FILE",
                            kFileIndicators[i],
                            fullPath
                        );

                        if (IsVmEvidence(kFileIndicators[i]))
                            gated = true;
                    }
                }
            }

        } while (FindNextFileW(hFind, &data));

        FindClose(hFind);

        return gated;
    }

    bool ScanCommonDirectories()
    {
        bool gated = false;

        wchar_t windowsDir[MAX_PATH] = {};
        wchar_t programFiles[MAX_PATH] = {};
        wchar_t programFilesX86[MAX_PATH] = {};

        GetWindowsDirectoryW(
            windowsDir,
            MAX_PATH
        );

        GetEnvironmentVariableW(
            L"ProgramFiles",
            programFiles,
            MAX_PATH
        );

        GetEnvironmentVariableW(
            L"ProgramFiles(x86)",
            programFilesX86,
            MAX_PATH
        );

        // Windows itself + drivers.
        gated |= ScanDirectory(windowsDir);

        wchar_t drivers[MAX_PATH];

        swprintf(
            drivers,
            MAX_PATH,
            L"%s\\System32\\drivers",
            windowsDir
        );

        gated |= ScanDirectory(drivers);

        // Common program installation locations.
        if (programFiles[0])
            gated |= ScanDirectory(programFiles);

        if (programFilesX86[0])
            gated |= ScanDirectory(programFilesX86);

        return gated;
    }

    // ============================================================
    // 3. Process indicators
    // ============================================================

    const wchar_t* kProcessIndicators[] =
    {
        // Generic
        L"sample.exe",
        L"bot.exe",
        L"sandbox.exe",
        L"malware.exe",
        L"test.exe",
        L"klavme.exe",
        L"myapp.exe",
        L"testapp.exe",

        // VirtualBox
        L"vboxservice.exe",
        L"vboxtray.exe",
        L"VBoxControl.exe",

        // Virtual PC
        L"vmusrvc.exe",
        L"vmsrvc.exe",
        L"VMSrvc.exe",
        L"VMUSrvc.exe",

        // VMware
        L"vmtoolsd.exe",
        L"vmwaretray.exe",
        L"vmwareuser.exe",
        L"VGAuthService.exe",
        L"vmacthlp.exe",
        L"vmwareservice.exe",
        L"vgautservice.exe",

        // QEMU
        L"qemu-ga.exe",

        // Xen
        L"xenservice.exe",

        // Parallels
        L"prl_cc.exe",
        L"prl_tools.exe",
    };

    const size_t kProcessCount =
        sizeof(kProcessIndicators) /
        sizeof(kProcessIndicators[0]);

    bool ScanProcesses()
    {
        bool gated = false;

        HANDLE snapshot =
            CreateToolhelp32Snapshot(
                TH32CS_SNAPPROCESS,
                0
            );

        if (snapshot == INVALID_HANDLE_VALUE)
            return false;

        PROCESSENTRY32W entry;
        entry.dwSize = sizeof(entry);

        if (!Process32FirstW(snapshot, &entry))
        {
            CloseHandle(snapshot);
            return false;
        }

        do
        {
            for (size_t i = 0; i < kProcessCount; ++i)
            {
                if (EqualsICase(
                        entry.szExeFile,
                        kProcessIndicators[i]))
                {
                    Report(
                        L"PROCESS",
                        kProcessIndicators[i],
                        entry.szExeFile
                    );

                    if (IsVmEvidence(kProcessIndicators[i]))
                        gated = true;
                }
            }

        } while (Process32NextW(snapshot, &entry));

        CloseHandle(snapshot);

        return gated;
    }

    // ============================================================
    // 4. Loaded DLL indicators
    // ============================================================

    const wchar_t* kModuleIndicators[] =
    {
        // AVG
        L"avghookx.dll",
        L"avghooka.dll",

        // Avast
        L"snxhk.dll",

        // Sandbox
        L"pstorec.dll",
        L"vmcheck.dll",
        L"wpespy.dll",

        // Comodo
        L"cmdvrt64.dll",
        L"cmdvrt32.dll",

        // VirtualBox
        L"vboxdisp.dll",
        L"vboxhook.dll",
        L"vboxmrxnp.dll",
        L"vboxogl.dll",

        // VMware
        L"vmGuestLib.dll",
        L"vm3dgl.dll",
        L"vmdum.dll",
        L"vm3dver.dll",
        L"vmtray.dll",
        L"VMToolsHook.dll",
        L"vmmousever.dll",
        L"VmGuestLibJava.dll",
    };

    const size_t kModuleCount =
        sizeof(kModuleIndicators) /
        sizeof(kModuleIndicators[0]);

    bool ScanLoadedModules()
    {
        bool gated = false;

        for (size_t i = 0; i < kModuleCount; ++i)
        {
            HMODULE module =
                GetModuleHandleW(kModuleIndicators[i]);

            if (module != NULL)
            {
                Report(
                    L"MODULE",
                    kModuleIndicators[i],
                    L"loaded in current process"
                );

                if (IsVmEvidence(kModuleIndicators[i]))
                    gated = true;
            }
        }

        return gated;
    }

    // ============================================================
    // 5. CPUID hypervisor indicators
    // ============================================================

    const wchar_t* kCpuidIndicators[] =
    {
        L"KVMKVMKVM",
        L"Microsoft Hv",
        L"VMwareVMware",
        L"VBoxVBoxVBox",
        L"XenVMMXenVMM",
        L"TCGTCGTCG",
    };

    bool ScanCpuid()
    {
        int cpuInfo[4] = {};

        __cpuid(cpuInfo, 1);

        // ECX bit 31 = hypervisor-present.
        if (!(cpuInfo[2] & (1 << 31)))
        {
            wprintf(L"[CPUID] Hypervisor-present bit: NO\n");
            return false;
        }

        wprintf(L"[CPUID] Hypervisor-present bit: YES\n");

        __cpuid(cpuInfo, 0x40000000);

        char raw[13];

        memcpy(raw + 0, &cpuInfo[1], 4); // EBX
        memcpy(raw + 4, &cpuInfo[2], 4); // ECX
        memcpy(raw + 8, &cpuInfo[3], 4); // EDX

        raw[12] = '\0';

        wchar_t vendor[13];

        for (int i = 0; i < 13; ++i)
            vendor[i] = static_cast<wchar_t>(
                static_cast<unsigned char>(raw[i])
            );

        wprintf(L"[CPUID] Vendor: %s\n", vendor);

        for (size_t i = 0;
             i < (sizeof(kCpuidIndicators) /
                  sizeof(kCpuidIndicators[0]));
             ++i)
        {
            if (ContainsICase(
                    vendor,
                    kCpuidIndicators[i]))
            {
                Report(
                    L"CPUID",
                    kCpuidIndicators[i],
                    vendor
                );
            }
        }

        // CPUID vendor strings are definitive VM evidence.
        return true;
    }

    // ============================================================
    // 6. Device / named-object indicators
    // ============================================================

    const wchar_t* kDeviceIndicators[] =
    {
        // VirtualBox
        L"\\\\.\\VBoxMiniRdrDN",
        L"\\\\.\\VBoxGuest",
        L"\\\\.\\pipe\\VBoxMiniRdDN",
        L"\\\\.\\VBoxTrayIPC",
        L"\\\\.\\pipe\\VBoxTrayIPC",

        // VMware
        L"\\\\.\\HGFS",
        L"\\\\.\\vmci",
    };

    const size_t kDeviceCount =
        sizeof(kDeviceIndicators) /
        sizeof(kDeviceIndicators[0]);

    bool ScanDevices()
    {
        bool gated = false;

        for (size_t i = 0; i < kDeviceCount; ++i)
        {
            HANDLE h =
                CreateFileW(
                    kDeviceIndicators[i],
                    0,
                    FILE_SHARE_READ |
                    FILE_SHARE_WRITE |
                    FILE_SHARE_DELETE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );

            if (h != INVALID_HANDLE_VALUE)
            {
                Report(
                    L"DEVICE",
                    kDeviceIndicators[i],
                    L"opened successfully"
                );

                gated = true;

                CloseHandle(h);
            }
            else
            {
                DWORD error = GetLastError();

                // ACCESS_DENIED can mean the object exists but
                // the current process cannot open it.
                if (error == ERROR_ACCESS_DENIED)
                {
                    Report(
                        L"DEVICE",
                        kDeviceIndicators[i],
                        L"exists/blocked by access control"
                    );

                    gated = true;
                }
            }
        }

        return gated;
    }

    // ============================================================
    // 7. VirtualBox window indicators
    // ============================================================

    bool ScanWindows()
    {
        bool gated = false;

        HWND hwnd =
            FindWindowW(
                L"VBoxTrayToolWndClass",
                NULL
            );

        if (hwnd != NULL)
        {
            Report(
                L"WINDOW",
                L"VBoxTrayToolWndClass",
                L"window class found"
            );

            gated = true;
        }

        hwnd =
            FindWindowW(
                NULL,
                L"VBoxTrayToolWnd"
            );

        if (hwnd != NULL)
        {
            Report(
                L"WINDOW",
                L"VBoxTrayToolWnd",
                L"window title found"
            );

            gated = true;
        }

        return gated;
    }
}

bool RunVmScan()
{
    wprintf(L"===============================================\n");
    wprintf(L" Windows VM / Sandbox Indicator Scanner\n");
    wprintf(L" Diagnostic mode - no evasion behavior\n");
    wprintf(L"===============================================\n\n");

    bool found = false;

    wprintf(L"[1/7] Registry\n");
    found = ScanRegistryTree(L"HARDWARE", 0);
    found = ScanRegistryTree(L"SYSTEM", 0) || found;
    found = ScanRegistryTree(L"SOFTWARE", 0) || found;

    wprintf(L"\n[2/7] Filesystem\n");
    found = ScanCommonDirectories() || found;

    wprintf(L"\n[3/7] Processes\n");
    found = ScanProcesses() || found;

    wprintf(L"\n[4/7] Loaded modules\n");
    found = ScanLoadedModules() || found;

    wprintf(L"\n[5/7] CPUID\n");
    found = ScanCpuid() || found;

    wprintf(L"\n[6/7] Devices / pipes\n");
    found = ScanDevices() || found;

    wprintf(L"\n[7/7] Windows\n");
    found = ScanWindows() || found;

    wprintf(L"\nDone.\n");

    return found;
}