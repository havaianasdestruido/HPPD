#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <shlobj.h>

#include "desk.h"

#ifdef _MSC_VER
#pragma comment(lib, "shell32.lib")
#endif

// ============================================================
// Append the ominous line to one .log file (never overwrite).
// Written as UTF-8 so Notepad shows it cleanly.
// ============================================================

static void AppendLine(const wchar_t* path)
{
    SYSTEMTIME st;
    GetLocalTime(&st);

    wchar_t line[256];

    wsprintfW(
        line,
        L"\n[%02u/%02u/%04u] - [%02u:%02u:%02u] - "
        L"something bad is about to happen..",
        st.wDay,
        st.wMonth,
        st.wYear,
        st.wHour,
        st.wMinute,
        st.wSecond
    );

    HANDLE file =
        CreateFileW(
            path,
            FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

    if (file == INVALID_HANDLE_VALUE)
        return;

    char utf8[512];

    int len =
        WideCharToMultiByte(
            CP_UTF8,
            0,
            line,
            -1,
            utf8,
            sizeof(utf8),
            NULL,
            NULL
        );

    if (len > 1)
    {
        DWORD written;

        WriteFile(
            file,
            utf8,
            len - 1,
            &written,
            NULL
        );
    }

    CloseHandle(file);
}

// ============================================================
// Scan the current user's desktop for *.log files.
// ============================================================

static void LogDesktop()
{
    wchar_t desktop[MAX_PATH];

    if (
        SHGetFolderPathW(
            NULL,
            CSIDL_DESKTOPDIRECTORY,
            NULL,
            SHGFP_TYPE_CURRENT,
            desktop
        ) != S_OK
    )
    {
        return;
    }

    wchar_t pattern[MAX_PATH];

    wsprintfW(
        pattern,
        L"%ls\\*.log",
        desktop
    );

    WIN32_FIND_DATAW fd{};

    HANDLE find =
        FindFirstFileW(pattern, &fd);

    if (find == INVALID_HANDLE_VALUE)
        return;

    do
    {
        wchar_t path[MAX_PATH];

        wsprintfW(
            path,
            L"%ls\\%ls",
            desktop,
            fd.cFileName
        );

        AppendLine(path);
    }
    while (FindNextFileW(find, &fd));

    FindClose(find);
}

void EasterDesk()
{
    LogDesktop();
}