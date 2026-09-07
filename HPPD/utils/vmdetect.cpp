#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#include <intrin.h>
#include <wctype.h>

#include "vmdetect.h"

// Case-insensitive substring search.
static bool ContainsICase(
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

// Vendor/model fragments (case-insensitive) that typically
// identify a virtual machine platform. Mirrors the common WMI
// checks but reads the same SMBIOS data from the registry
// (works on XP, no WMI dependency).
static const wchar_t* kVmStrings[] =
{
    L"VMware",
    L"VirtualBox",
    L"innotek",             // VirtualBox's original vendor name
    L"Oracle",              // VirtualBox vendor since 4.x
    L"Microsoft Corporation",// Hyper-V / Virtual PC baseboard vendor
    L"Virtual Machine",     // Hyper-V / Virtual PC system model
    L"QEMU",
    L"KVM",
    L"Red Hat",
    L"Xen",
    L"HVM domU",
    L"Parallels",
    L"Bochs",
};

// Reads a REG_SZ from HKLM\HARDWARE\DESCRIPTION\System\BIOS
// (the registry view of the SMBIOS tables).
static bool ReadBiosString(
    const wchar_t* valueName,
    wchar_t* out,
    int outCch
)
{
    HKEY key = NULL;

    if (
        RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"HARDWARE\\DESCRIPTION\\System\\BIOS",
            0,
            KEY_READ,
            &key
        ) != ERROR_SUCCESS
    )
        return false;

    DWORD size = (DWORD)outCch * sizeof(wchar_t);

    LONG rc =
        RegQueryValueExW(
            key,
            valueName,
            NULL,
            NULL,
            (LPBYTE)out,
            &size
        );

    RegCloseKey(key);

    return rc == ERROR_SUCCESS && out[0] != L'\0';
}

// Non-privileged check: when the CPUID "hypervisor present" bit
// is set, a hypervisor (and thus a VM) is running underneath.
// (Consumes the technique supplied as reference.)
static bool CpuHypervisorPresent()
{
    int cpuInfo[4] = {};

    __cpuid(cpuInfo, 1);

    return ((cpuInfo[2] >> 31) & 1) == 1;
}

// SMBIOS vendor/model strings usually name the VM platform.
static bool SmbiosLooksLikeVm()
{
    wchar_t systemManufacturer[256] = {};
    wchar_t systemProductName[256] = {};
    wchar_t baseboardManufacturer[256] = {};

    ReadBiosString(
        L"SystemManufacturer",
        systemManufacturer,
        256
    );

    ReadBiosString(
        L"SystemProductName",
        systemProductName,
        256
    );

    ReadBiosString(
        L"BaseBoardManufacturer",
        baseboardManufacturer,
        256
    );

    for (const wchar_t* sig : kVmStrings)
    {
        if (ContainsICase(systemManufacturer, sig))
            return true;

        if (ContainsICase(systemProductName, sig))
            return true;

        if (ContainsICase(baseboardManufacturer, sig))
            return true;
    }

    return false;
}

bool IsVirtualMachine()
{
    if (CpuHypervisorPresent())
        return true;

    if (SmbiosLooksLikeVm())
        return true;

    return false;
}