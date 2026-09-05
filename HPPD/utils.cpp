#include "utils.h"

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
