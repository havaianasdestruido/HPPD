# HPPD — Detection & Incident Response Notes

Analysis of the `havaianasdestruido/HPPD` GDI trojan. All indicators below were
extracted verbatim from the source tree; nothing here is inferred from the
project README, which overstates and understates in different places.

## Artifacts in this directory

| File | Purpose |
|---|---|
| `hppd.yara` | 10 YARA rules. Every string literal verified to exist in the source. |
| `Detect-HppdArtifacts.ps1` | Host triage script. Read-only unless `-Remediate` + token. |
| `iocs.csv` | Machine-readable IOC list with source line references. |

## Validation performed

- All 38 YARA string literals traced back to `HPPD/**` — no invented indicators.
- The 43-entry IFEO target list in the triage script diffed against
  `HPPD/regfuk.cpp` — exact match, no drift in either direction.
- Brace/paren/bracket balance checked on both scripts.

## Behaviour summary

### Persistence — IFEO mass hijack
`HPPD/regfuk.cpp` writes `HKLM\...\Image File Execution Options\<target>\Debugger`
= `wscript.exe C:\Users\Public\ghostroot\Message.vbs` for **43** binaries.

The target list is chosen deliberately to maximise damage to the victim:

- **Security tools** — `msert.exe`, `mssecse.exe`, `mbam.exe`, `adwcleaner_5.005.exe`,
  `MSASCui.exe`, `CCleaner*.exe`, `mdsched.exe`
- **Analysis tools** — `autoruns.exe`, `procxp.exe`, `VirtualBox.exe`
- **System tools** — `cmd.exe`, `regedit.exe`, `taskmgr.exe`, `mmc.exe`,
  `explorer.exe`, `wmic.exe`, `msconfig.exe`, `takeown.exe`, `icacls.exe`, `attrib.exe`

That combination is the strongest single detection signal: any *one* IFEO
`Debugger` value is worth a look, and an IFEO hit on an AV or autoruns binary
should be treated as confirmed compromise.

### Host takeover
- `SafeBoot\AlternateShell` = `notepad.exe` — survives into recovery environment.
- `Policies\Explorer\RestrictRun` allow-lists (HKLM + HKCU) gate which binaries
  run at all, including `rpdbfk.exe`.
- `EnableLUA=0`, `DisableTaskMgr=1`, `DisableCMD=1`, `DisableUNCCheck=1`
  — UAC off, Task Manager gone, no console, UNC paths uncopyable.
- `NoDrives=67108863` (`0x03FFFFFF`) — every drive hidden in Explorer.
- `NoFind`, `NoRun`, `NoTrayContextMenu`, `NoTrayItemsDisplay`, `NoSetTaskbar`,
  `TaskbarNoPinnedList`, `NoPinningToTaskbar`, `NoSaveSettings` — shell stripped.
- `legalnoticecaption` / `legalnoticetext` — attacker text on every logon.
- `ComputerName` = `#OWNED by HPPD`.

### Digit spoofing
`HKCU\Control Panel\International\s1159` and `s2359` = `8==D`;
`sNativeDigits` = a repeated Unicode hieroglyph sequence. Affects digit display
in Calculator and Excel — an easy way for an analyst to notice something is
wrong even if they don't know what they're looking for.

### File infection (payload 68)
Recursive mass infection of `*.bat`, `*.cmd`, `*.com`, `*.pdf`, `*.ps1`:

- **PDF** — injects `/Type /Catalog /OpenAction` with a JavaScript action
  (`app.alert('THE END IS NEAR ...')`) rendered via `/S /JavaScript /JS`.
- **PowerShell** — prepends `Stop-Computer -Force` under
  `powershell -WindowStyle Hidden -Command`.
- **Batch** — appends `:: backward compatibility trick`.
- Text files additionally corrupted with Unicode via `utils/unicurse.h`.

Infected documents are **not** cleanly reversible — recovery means restoring from
backup.

### Anti-analysis
`HPPD/rules.cpp` embeds a catalog of ~40 self-describing rule names
(`check-for-peb-beingdebugged-flag`, `check-for-sandbox-and-av-modules`,
`reference-anti-vm-strings`, …). These are distinctive literals in a PE and are
easy to match on. `utils/vmdetect.cpp` adds VM/sandbox fingerprinting
(`VirtualBox.exe`, VMware artefacts, `BXPC`, `BOCHS`, `06/23/99`).

`utils/critical.cpp` sets `ProcessBreakOnTermination` (info class `0x1D`) via
`NtSetInformationProcess`, making the process **critical** — killing it bugchecks
the machine. Triage must not kill the process.

### Anti-forensics note
`HPPD/main.cpp:33-37` claims "No disk writes, no MBR modification". The code
contradicts the README and that comment: `regfuk.cpp`, `rules.cpp`,
`critical.cpp` and the `utils/fileinfect` module all perform substantial
system modification and disk writes. Treat the source comments as unreliable;
verify against code.

## ATT&CK mapping

| Technique | ID | Evidence |
|---|---|---|
| IFEO Injection | T1546.015 | 43 `Debugger` writes |
| Modify Registry | T1112 | `regfuk.cpp` in full |
| Impair Defenses | T1562.001 | Disable Task Manager, `EnableLUA=0`, `DisableCMD` |
| Inhibit System Recovery | T1490 | `SafeBoot\AlternateShell` |
| Internal Defacement | T1491.001 | `legalnoticecaption`/`text` |
| Data Encrypted for Impact | T1486 | `Stop-Computer -Force` in `.ps1` |
| Resource Hijacking | T1496 | mass document infection |
| Ingress Tool Transfer | T1105 | `ghostroot\` staging, `rpdbfk.exe` |
| System Information Discovery | T1082 | `rules.cpp` |
| Process / Module Discovery | T1057 | `rules.cpp` |
| Query Registry | T1012 | `rules.cpp`, `Reg::` helpers |
| Network Config / Connections / Remote System | T1016 / T1049 / T1018 | `rules.cpp` |
| File and Directory Discovery | T1083 | `rules.cpp` |
| Virtualization/Sandbox Evasion | T1497.001 | `vmdetect.cpp` |
| Debugger Evasion | T1622 | PEB `BeingDebugged`, breakpoint scans |
| Endpoint DoS | T1499 | critical-process watchdog |

## Response order of operations

1. **Do not terminate the process.** `ProcessBreakOnTermination` is likely set.
2. Capture memory and the running binary before touching disk.
3. Disconnect from network.
4. Run `Detect-HppdArtifacts.ps1` elevated for a full artifact inventory
   (report-only first).
5. **Assume reimage.** IFEO across AV tooling, SafeBoot hijack, RestrictRun, and
   mass document infection are not cleanly reversible, and the AV binaries it
   hijacks are the tools you would use to verify a clean state.
6. Restore documents from backup. Do not attempt to "clean" infected files —
   the content is attacker-authored.

## Detection notes

`hppd.yara` rules 3, 5, 6 and 10 are the useful ones for production. Rule 9
(author taunts) is informational and pivoting-only — too short and too
generic to alert on. Rule 7 (`Digit_Spoofing`) is medium severity: `s1159` and
`s2359` do not exist in stock Windows, so the key names alone are specific, but
the rule is narrow by design.

The GDI effects payloads (1–63, 69–72) are per-pixel demoscene renderers drawing
to a memory DC. They are not malicious on their own and are not included as
detection indicators, though a compiled binary containing all of them plus the
registry strings above is unambiguously this sample.