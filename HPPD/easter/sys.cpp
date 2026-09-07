#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#include "sys.h"

// ============================================================
// Pirated-Windows heuristics (XP era).
//
// HasWgaLogon: Microsoft's Genuine Advantage notification
//   engine registered under Winlogon/Notify after a failed
//   genuine check.
//
// OobetimerTampered: the WPAEvents activation timer either
//   lost its value or was zeroed out by a crack/keygen.
//
// HasFckgwPID: the ProductID contains the infamous leaked
//   volume-license "FCKGW" block.
// ============================================================

static bool HasWgaLogon()
{
    HKEY key = NULL;

    if (
        RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows NT\\CurrentVersion\\"
            L"Winlogon\\Notify\\WgaLogon",
            0,
            KEY_READ,
            &key
        ) != ERROR_SUCCESS
    )
    {
        return false;
    }

    RegCloseKey(key);
    return true;
}

static bool OobetimerTampered()
{
    HKEY key = NULL;

    // Key absent (e.g. modern Windows) => nothing to flag.
    if (
        RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows NT\\CurrentVersion\\WPAEvents",
            0,
            KEY_READ,
            &key
        ) != ERROR_SUCCESS
    )
    {
        return false;
    }

    BYTE data[64];
    DWORD size = sizeof(data);

    LONG result =
        RegQueryValueExW(
            key,
            L"OOBETimer",
            NULL,
            NULL,
            data,
            &size
        );

    RegCloseKey(key);

    if (result != ERROR_SUCCESS || size == 0)
        return true;

    for (DWORD i = 0; i < size; ++i)
    {
        if (data[i] != 0)
            return false;
    }

    return true;
}

static bool HasFckgwPID()
{
    HKEY key = NULL;

    if (
        RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows NT\\CurrentVersion",
            0,
            KEY_READ,
            &key
        ) != ERROR_SUCCESS
    )
    {
        return false;
    }

    wchar_t pid[128];
    DWORD size = sizeof(pid);

    LONG result =
        RegQueryValueExW(
            key,
            L"ProductID",
            NULL,
            NULL,
            reinterpret_cast<LPBYTE>(pid),
            &size
        );

    RegCloseKey(key);

    if (result != ERROR_SUCCESS || size < sizeof(wchar_t))
        return false;

    pid[size / sizeof(wchar_t)] = L'\0';

    return wcsstr(pid, L"FCKGW") != NULL;
}

static bool IsPirated()
{
    return
        HasFckgwPID() ||
        HasWgaLogon() ||
        OobetimerTampered();
}

void EasterSys()
{
    if (!IsPirated())
        return;

    MessageBoxW(
        NULL,
        L"looks like someones too poor to get its computer fried.",
        L"hahahaha",
        MB_OK |
        MB_ICONINFORMATION
    );
}