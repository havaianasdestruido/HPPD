#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#include "payloads.h"

// ============================================================
// Safe visual-only GDI effects demo
//
// No:
// - disk writes
// - MBR modification
// - privilege escalation
// - BSOD / NtRaiseHardError
// - audio playback
//
// ESC aborts Payloads 13-24.
// Payloads 1-12 retain their original finite durations.
// ============================================================

// ============================================================
// Existing helper
// ============================================================

BOOL CALLBACK EnumProc(HWND hWnd, LPARAM)
{
    ShowWindow(hWnd, SW_SHOW);
    return TRUE;
}

// ============================================================
// EFFECT RUNNER
// ============================================================

void runEffect(
    LPTHREAD_START_ROUTINE effect
)
{
    HANDLE thread =
        CreateThread(
            NULL,
            0,
            effect,
            NULL,
            0,
            NULL
        );

    if (!thread)
        return;

    WaitForSingleObject(
        thread,
        INFINITE
    );

    CloseHandle(thread);

    RedrawWindow(
        NULL,
        NULL,
        NULL,
        RDW_INVALIDATE |
        RDW_ERASE |
        RDW_UPDATENOW |
        RDW_ALLCHILDREN
    );

    Sleep(250);
}

// ============================================================
// MAIN
// ============================================================

int main()
{
    int result =
        MessageBoxW(
            NULL,
            L"This is a visual-effects demo.\r\n\r\n"
            L"It does not modify your disk, crash Windows, "
            L"change privileges, or play audio.\r\n\r\n"
            L"Press ESC during Payloads 13-24 "
            L"to skip the current effect.\r\n\r\n"
            L"Continue?",
            L"Bromine Visual Demo",
            MB_YESNO |
            MB_ICONINFORMATION
        );

    if (result != IDYES)
        return 0;

    // --------------------------------------------------------
    // Original Payloads
    // --------------------------------------------------------

    /*runEffect(GDIPayload1);
    runEffect(GDIPayload2);
    runEffect(GDIPayload3);
    runEffect(GDIPayload4);
    runEffect(GDIPayload5);
    runEffect(GDIPayload6);
    runEffect(GDIPayload7);
    runEffect(GDIPayload8);
    runEffect(GDIPayload9);
    runEffect(GDIPayload10);
    runEffect(GDIPayload11);
    runEffect(GDIPayload12);*/

    // --------------------------------------------------------
    // New high-performance Payloads
    // --------------------------------------------------------

    runEffect(GDIPayload13);
    runEffect(GDIPayload14);
    runEffect(GDIPayload15);
    runEffect(GDIPayload16);
    runEffect(GDIPayload17);
    runEffect(GDIPayload18);
    runEffect(GDIPayload19);
    runEffect(GDIPayload20);
    runEffect(GDIPayload21);
    runEffect(GDIPayload22);
    runEffect(GDIPayload23);
    runEffect(GDIPayload24);

    // --------------------------------------------------------
    // Final cleanup
    // --------------------------------------------------------

    RedrawWindow(
        NULL,
        NULL,
        NULL,
        RDW_INVALIDATE |
        RDW_ERASE |
        RDW_UPDATENOW |
        RDW_ALLCHILDREN
    );

    MessageBoxW(
        NULL,
        L"Visual demo finished.",
        L"Bromine Visual Demo",
        MB_OK |
        MB_ICONINFORMATION
    );

    return 0;
}
