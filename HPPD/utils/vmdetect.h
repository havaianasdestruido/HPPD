#pragma once

// Best-effort virtual-machine detection.
//
// Returns true when the machine reports signs of a hypervisor
// (CPUID hypervisor-present bit, or SMBIOS vendor/model strings
// that name a known VM platform). Returns false on bare metal —
// and also when it cannot tell (checks fail safe to false).
//
// Caveat: on Windows 10/11 with Virtualization-Based Security
// (Core Isolation / Device Guard / WSL2, etc.) the Hyper-V
// hypervisor is present on PHYSICAL machines too, so the CPUID
// bit alone can read "VM" on real hardware.
bool IsVirtualMachine();