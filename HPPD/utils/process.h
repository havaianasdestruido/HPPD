#pragma once

#include <windows.h>

// ============================================================
// Process manipulation.
//
// All suspend/resume work goes through the documented Toolhelp
// thread enumeration + SuspendThread/ResumeThread, which behave
// identically on XP and W10. Kick prints that require rights to
// the target; system-critical processes refuse. The chaos helpers
// always restore NORMAL priority before returning.
// ============================================================

namespace Process {

// Print process ID, thread count and name for every process.
void List();

// Index of the first process whose image name matches, or -1.
// Matches by full name (the snapshot truncates EXE names to 15
// chars on XP, so pass the short name).
int FindIndex(const wchar_t* name);

// Suspend / resume a process (by index from FindIndex, or by
// numerical pid).
bool Suspend(int index);
bool Resume(int index);
bool SuspendPid(DWORD pid);
bool ResumePid(DWORD pid);

// Suspend / resume by image name; first match wins.
bool SuspendByName(const wchar_t* name);
bool ResumeByName(const wchar_t* name);

// Terminate every process matching the name. Returns count killed.
int KillByName(const wchar_t* name);

// Chaos: random priority class every ~120ms for `ms`, then NORMAL.
void PriorityChaos(const wchar_t* name, DWORD ms);

}