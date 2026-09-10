#pragma once

// Deep virtual-machine / sandbox scan, seven categories:
//
//   1. Registry (recursive HKLM\HARDWARE, HKLM\SYSTEM, HKLM\SOFTWARE)
//   2. Filesystem (Windows, drivers, Program Files trees)
//   3. Processes
//   4. Loaded modules
//   5. CPUID hypervisor vendor
//   6. Devices / named pipes
//   7. VirtualBox tray windows
//
// RunVmScan() prints one "[FOUND] category indicator -> where"
// line per hit (console diagnostic, like the original reference
// scanner) and returns true when at least one hit is genuine
// VM-platform evidence (VirtualBox / VMware / QEMU / Bochs /
// Hyper-V / Xen / Parallels / KVM, CPUID vendor, devices,
// VBoxTray windows). Sandbox and AV fingerprints plus generic
// executable names are reported but do NOT flip the return value.
//
// The startup VM gate in main.cpp uses it as a reinforcement
// layer only when the fast utils/vmdetect.cpp checks come back
// negative (it is meant to drill deeper on inconclusive hosts).
bool RunVmScan();