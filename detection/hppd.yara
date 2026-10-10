/*
    HPPD (Hallucinogen Persisting Perception Disorder) - GDI trojan
    Detection rules derived from source review of the public repository.

    All strings below were extracted verbatim from the source tree
    (HPPD/regfuk.cpp, HPPD/rules.cpp, HPPD/utils/fileinfect/*,
     HPPD/main.cpp, HPPD/utils/critical.cpp).

    Triage:  yara -s hppd.yara <sample>
    Author:  Arena defensive analysis
*/

import "pe"

/* ---------------------------------------------------------------- *
 * 1. High-confidence: the "#OWNED by HPPD" marker plus any other
 *    HPPD-specific artifact. No legitimate software emits these.
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_Owned_Marker
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - attacker ownership marker string"
        severity    = "critical"
        source      = "HPPD/regfuk.cpp:153,158,163 (ComputerName, legalnoticecaption/text)"
        ttp         = "T1565.001 / defacement-style host marker"

    strings:
        $marker = "#OWNED by HPPD" ascii wide
        $lawful = "someone wants to escape from me" ascii wide

    condition:
        $marker
}

/* ---------------------------------------------------------------- *
 * 2. Dropper staging paths and the dropped payload name.
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_Dropper_Artifacts
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - ghostroot staging directory and rpdbfk.exe"
        severity    = "critical"
        source      = "HPPD/regfuk.cpp:195-520"
        ttp         = "T1105 / T1547 persistence staging"

    strings:
        $ghost   = "ghostroot\\Message.vbs" ascii wide nocase
        $ghost2  = "ghostroot" ascii wide
        $rpdbfk  = "rpdbfk.exe" ascii wide
        $wscript = "wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs" ascii wide

    condition:
        2 of them
}

/* ---------------------------------------------------------------- *
 * 3. IFEO hijack batch. The malware writes an identical Debugger value
 *    under HKLM\...\Image File Execution Options\<target>\Debugger for
 *    43 targets. We match the key family plus the debugger payload.
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_IFEO_Persistence
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - Image File Execution Options mass hijack"
        severity    = "critical"
        source      = "HPPD/regfuk.cpp (43 IFEO Debugger writes)"
        ttp         = "T1546.015 Image File Execution Options Injection"
        mitre       = "https://attack.mitre.org/techniques/T1546/015/"

    strings:
        $ifeo = "Image File Execution Options" ascii wide
        $dbg  = "Debugger" ascii wide
        $pay  = "wscript.exe C:\\Users\\Public\\ghostroot\\Message.vbs" ascii wide

    condition:
        all of them
}

/* ---------------------------------------------------------------- *
 * 4. File-infection payload bodies (PDF / batch / PowerShell).
 *    A victim machine shows these strings inside user documents.
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_File_Infector
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - mass file infector payload fragments"
        severity    = "high"
        source      = "HPPD/utils/fileinfect/{pdf,batchfile,powershell}.cpp"
        ttp         = "T1486 / T1496 resource hijacking of user documents"

    strings:
        $pdfjs   = "app.alert('THE END IS NEAR" ascii wide
        $pdfjs2  = "/S /JavaScript /JS" ascii wide
        $pdfcat  = "/Type /Catalog /OpenAction" ascii wide
        $ps      = "Stop-Computer -Force" ascii wide
        $ps2     = "powershell -WindowStyle Hidden -Command" ascii wide
        $bat     = "backward compatibility trick" ascii wide

    condition:
        any of them
}

/* ---------------------------------------------------------------- *
 * 5. Reconnaissance rule-catalog strings.
 *    rules.cpp embeds its own snake_case rule names as literals.
 *    A short-form table of agent-framework labels in a PE is a strong
 *    signal on its own.
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_Recon_Rulecatalog
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - embedded reconnaissance rule catalog"
        severity    = "high"
        source      = "HPPD/rules.cpp:94-616"
        ttp         = "T1082 / T1057 / T1012 / T1016 / T1049 / T1018 discovery"

    strings:
        $r1 = "check-for-peb-beingdebugged-flag" ascii wide
        $r2 = "check-for-time-delay-via-gettickcount" ascii wide
        $r3 = "check-for-sandbox-username-or-hostname" ascii wide
        $r4 = "check-for-sandbox-and-av-modules" ascii wide
        $r5 = "reference-anti-vm-strings" ascii wide
        $r6 = "check-for-hardware-breakpoints" ascii wide

    condition:
        3 of them
}

/* ---------------------------------------------------------------- *
 * 6. Registry destruction / host-lockdown bundle.
 *    Individually each value is mundane; together they are HPPD.
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_Registry_Lockdown
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - explorer lockdown and SafeBoot shell replacement"
        severity    = "high"
        source      = "HPPD/regfuk.cpp:68-150"
        ttp         = "T1562.001 impair defenses, T1490, T1112"

    strings:
        $safeboot = "SafeBoot\\AlternateShell" ascii wide
        $restrun  = "Policies\\Explorer\\RestrictRun" ascii wide
        $nofind   = "Policies\\Explorer\\NoFind" ascii wide
        $norun    = "Policies\\Explorer\\NoRun" ascii wide
        $nodrv    = "Policies\\Explorer\\NoDrives" ascii wide
        $lua      = "Policies\\System\\EnableLUA" ascii wide
        $discmd   = "Policies\\System\\DisableCMD" ascii wide
        $notray   = "NoTrayContextMenu" ascii wide

    condition:
        4 of them
}

/* ---------------------------------------------------------------- *
 * 7. Digit spoofing (Unicode digit substitution).
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_Digit_Spoofing
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - s1159/s2359/sNativeDigits digit spoofing"
        severity    = "medium"
        source      = "HPPD/regfuk.cpp:172-186"
        ttp         = "T1112 modify registry"

    strings:
        $s1159 = "s1159" ascii wide
        $s2359 = "s2359" ascii wide
        $nat   = "sNativeDigits" ascii wide
        $val   = "8==D" ascii wide

    condition:
        ($s1159 and $s2359 and $nat) or ($s1159 and $val)
}

/* ---------------------------------------------------------------- *
 * 8. Critical-process watchdog (BSOD on kill).
 *    utils/critical.cpp sets ProcessBreakOnTermination (0x1D).
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_Critical_Process
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - ProcessBreakOnTermination watchdog"
        severity    = "high"
        source      = "HPPD/utils/critical.cpp:12,17,49"
        ttp         = "T1499 endpoint DoS / anti-termination"

    strings:
        $api  = "NtSetInformationProcess" ascii wide
        $priv = "SeDebugPrivilege" ascii wide

    condition:
        all of them and filesize < 20MB
}

/* ---------------------------------------------------------------- *
 * 9. Author taunts - low specificity, useful for pivoting.
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_Author_Taunts
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - author strings, for pivoting not alerting"
        severity    = "informational"
        source      = "HPPD/main.cpp:74-76"

    strings:
        $t1 = "Good luck reverse engineering this." ascii wide
        $t2 = "Go ahead and steal my payloads" ascii wide
        $t3 = "Hope you ruin all other people's fun." ascii wide

    condition:
        any of them
}

/* ---------------------------------------------------------------- *
 * 10. Combo rule - high confidence only, low false-positive budget.
 * ---------------------------------------------------------------- */
rule HPPD_Trojan_Combo_HighConfidence
{
    meta:
        author      = "Arena"
        date        = "2026-10-10"
        description = "HPPD GDI trojan - multi-artifact combination"
        severity    = "critical"
        reference   = "https://github.com/havaianasdestruido/HPPD"

    condition:
        filesize < 50MB and (
            (HPPD_Trojan_Owned_Marker) or
            (HPPD_Trojan_Dropper_Artifacts and 2 of (
                HPPD_Trojan_IFEO_Persistence,
                HPPD_Trojan_Registry_Lockdown,
                HPPD_Trojan_Recon_Rulecatalog,
                HPPD_Trojan_File_Infector
            )) or
            (3 of (
                HPPD_Trojan_IFEO_Persistence,
                HPPD_Trojan_Registry_Lockdown,
                HPPD_Trojan_Recon_Rulecatalog,
                HPPD_Trojan_File_Infector,
                HPPD_Trojan_Dropper_Artifacts,
                HPPD_Trojan_Digit_Spoofing
            ))
        )
}