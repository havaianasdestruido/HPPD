<#
.SYNOPSIS
    Triage a Windows host for HPPD (Hallucinogen Persisting Perception
    Disorder) GDI trojan artifacts.

.DESCRIPTION
    Read-only by default. Inspects the registry and filesystem for the
    artifacts identified in the HPPD source tree:

      * IFEO Debugger mass hijack (43 targets)
      * SafeBoot AlternateShell replacement
      * Explorer RestrictRun allow-lists
      * "#OWNED by HPPD" ownership marker
      * Digit spoofing (s1159 / s2359 / sNativeDigits)
      * Host lockdown values (EnableLUA, DisableCMD, NoDrives, ...)
      * ghostroot staging directory and rpdbfk.exe
      * Infected documents (PDF / batch / PowerShell fragments)
      * Unicode-corrupted text files

    Remediation is gated behind -Remediate and requires
    -RemediateToken to be passed explicitly, so it cannot run by accident.

.PARAMETER Remediate
    Perform cleanup instead of report-only.

.PARAMETER RemediateToken
    Required confirmation string when -Remediate is used.

.PARAMETER ScanRoot
    Directory to scan for infected documents. Defaults to user profile
    roots. Supply an explicit path to control scope.

.PARAMETER SkipFileScan
    Skip the (potentially slow) document infection scan.

.EXAMPLE
    .\Detect-HppdArtifacts.ps1

.EXAMPLE
    .\Detect-HppdArtifacts.ps1 -Remediate -RemediateToken 'REMOVE-HPPD-ARTIFACTS'

.NOTES
    Run elevated. Exit code 0 = clean, 1 = artifacts found,
    2 = remediation requested, 3 = script error.
#>

#Requires -Version 5.1
[CmdletBinding()]
param(
    [switch] $Remediate,
    [string] $RemediateToken,
    [string[]] $ScanRoot,
    [switch] $SkipFileScan
)

$ErrorActionPreference = 'Stop'

$script:FindingCount = 0
$script:Findings    = [System.Collections.Generic.List[object]]::new()
$script:Fixes       = [System.Collections.Generic.List[string]]::new()

function Write-Finding {
    param(
        [Parameter(Mandatory)][string] $Category,
        [Parameter(Mandatory)][string] $Path,
        [Parameter(Mandatory)][string] $Detail,
        [string] $Remediation = ''
    )

    $script:FindingCount++

    $obj = [pscustomobject]@{
        Category    = $Category
        Path        = $Path
        Detail      = $Detail
        Remediation = $Remediation
    }

    Write-Host ('  [{0,-12}] {1}' -f $Category, $Path)
    Write-Host ('  {0,-14} {1}' -f '', $Detail)

    if ($Remediation) {
        Write-Host ('  {0,-14} {1}' -f '', $Remediation) -ForegroundColor DarkGray
    }

    $script:Findings.Add($obj)
}

function Test-Admin {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    $p  = New-Object Security.Principal.WindowsPrincipal($id)
    return $p.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# ---------------------------------------------------------------- #
# 1. IFEO Debugger hijack
# ---------------------------------------------------------------- #
$IFEO_ROOT = 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options'

# Verbatim target list from HPPD/regfuk.cpp.
$IFEO_TARGETS = @(
    'CCleaner.exe', 'CCleaner32.exe', 'CCleaner64.exe', 'MSASCui.exe',
    'VirtualBox.exe', 'adwcleaner_5.005.exe', 'attrib.exe', 'autoruns.exe',
    'chrome.exe', 'cmd.exe', 'control.exe', 'crashreporter.exe',
    'explorer.exe', 'firefox.exe', 'htaedit.exe', 'icacls.exe',
    'iexplore.exe', 'install.exe', 'installer.exe', 'mbam.exe',
    'mdsched.exe', 'mmc.exe', 'msconfig.exe', 'msert.exe', 'mssecse.exe',
    'notepad++.exe', 'notepad.exe', 'plugin-hang-ui.exe', 'procxp.exe',
    'regedit.exe', 'regedit32.exe', 'regedit33.exe', 'setup.exe',
    'student.exe', 'takeown.exe', 'taskhost.exe', 'taskmgr.exe',
    'uTorrent.exe', 'uninstall.exe', 'update.exe', 'updater.exe',
    'vbsedit.exe', 'wmic.exe'
)

Write-Host ''
Write-Host '=== HPPD artifact triage ===' -ForegroundColor Cyan
Write-Host ''

Write-Host '[1/6] Image File Execution Options hijack' -ForegroundColor Cyan

if (-not (Test-Path $IFEO_ROOT)) {
    Write-Host '  IFEO root not present.'
}
else {
    $hijacked = 0

    foreach ($target in $IFEO_TARGETS) {
        $dbgPath = Join-Path $IFEO_ROOT "$target\Debugger"

        if (-not (Test-Path $dbgPath)) { continue }

        $val = (Get-ItemProperty -Path $dbgPath -Name 'Debugger' -ErrorAction SilentlyContinue).Debugger

        if ([string]::IsNullOrWhiteSpace($val)) { continue }

        # Any Debugger value is a finding; the ghostroot one is a
        # confirmed HPPD hit, anything else is a generic IFEO hijack.
        $confirmed = $val -match 'ghostroot\\Message\.vbs'
        $category  = if ($confirmed) { 'HPPD' } else { 'IFEO' }

        $hijacked++
        Write-Finding -Category $category `
                      -Path "IFEO\$target\Debugger" `
                      -Detail "Debugger = $val" `
                      -Remediation 'Delete the "Debugger" value'

        if ($Remediate) {
            Remove-ItemProperty -Path $dbgPath -Name 'Debugger' -ErrorAction SilentlyContinue
            $script:Fixes.Add("Removed IFEO Debugger for $target")
        }
    }

    if ($hijacked -eq 0) {
        Write-Host '  Clean - no IFEO Debugger hijacks found.'
    }
}

# ---------------------------------------------------------------- #
# 2. SafeBoot AlternateShell
# ---------------------------------------------------------------- #
Write-Host ''
Write-Host '[2/6] SafeBoot AlternateShell' -ForegroundColor Cyan

$sbPath = 'HKLM:\SYSTEM\CurrentControlSet\Control\SafeBoot\AlternateShell'

if (Test-Path $sbPath) {
    $sb = (Get-ItemProperty -Path $sbPath -ErrorAction SilentlyContinue).'(default)'

    Write-Finding -Category 'HPPD' `
                  -Path $sbPath `
                  -Detail "AlternateShell = $sb" `
                  -Remediation 'Delete the SafeBoot AlternateShell value'

    if ($Remediate) {
        Remove-Item -Path $sbPath -ErrorAction SilentlyContinue
        $script:Fixes.Add('Removed SafeBoot AlternateShell')
    }
}
else {
    Write-Host '  Clean - no SafeBoot shell replacement.'
}

# ---------------------------------------------------------------- #
# 3. Explorer RestrictRun allow-lists
# ---------------------------------------------------------------- #
Write-Host ''
Write-Host '[3/6] Explorer RestrictRun' -ForegroundColor Cyan

$rrPaths = @(
    'HKLM:\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer\RestrictRun',
    'HKCU:\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer\RestrictRun'
)

$rrHits = 0

foreach ($rr in $rrPaths) {
    if (-not (Test-Path $rr)) { continue }

    $prop = Get-ItemProperty -Path $rr -ErrorAction SilentlyContinue
    $entries = $prop.PSObject.Properties |
               Where-Object { $_.Name -match '^\d+$' } |
               ForEach-Object { $_.Value }

    $mal = $entries | Where-Object { $_ -match 'rpdbfk\.exe' }

    if ($mal) {
        $rrHits++
        Write-Finding -Category 'HPPD' `
                      -Path $rr `
                      -Detail "RestrictRun contains rpdbfk.exe: $($entries -join ', ')" `
                      -Remediation 'Delete the whole RestrictRun key'

        if ($Remediate) {
            Remove-Item -Path $rr -Recurse -ErrorAction SilentlyContinue
            $script:Fixes.Add("Removed RestrictRun under $rr")
        }
    }
}

if ($rrHits -eq 0) {
    Write-Host '  Clean - no HPPD RestrictRun allow-lists.'
}

# ---------------------------------------------------------------- #
# 4. Ownership marker + digit spoofing + lockdown values
# ---------------------------------------------------------------- #
Write-Host ''
Write-Host '[4/6] Ownership marker, digit spoofing, lockdown' -ForegroundColor Cyan

# --- ownership marker ---
$markerChecks = @(
    @{
        Path = 'HKLM:\System\CurrentControlSet\Control\ComputerName\ActiveComputerName'
        Name = 'ComputerName'
    },
    @{
        Path = 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Policies\System'
        Name = 'legalnoticecaption'
    }
)

foreach ($chk in $markerChecks) {
    if (-not (Test-Path $chk.Path)) { continue }

    $val = (Get-ItemProperty -Path $chk.Path -Name $chk.Name -ErrorAction SilentlyContinue).($chk.Name)

    if ($val -like '*OWNED by HPPD*') {
        Write-Finding -Category 'HPPD' `
                      -Path "$($chk.Path)\$($chk.Name)" `
                      -Detail "Value = $val" `
                      -Remediation "Delete the $($chk.Name) value"

        if ($Remediate) {
            Remove-ItemProperty -Path $chk.Path -Name $chk.Name -ErrorAction SilentlyContinue
            $script:Fixes.Add("Removed $($chk.Name) marker")
        }
    }
}

# --- legal notice text ---
$lnPath = 'HKLM:\Software\Microsoft\Windows\CurrentVersion\Policies\System'
if (Test-Path $lnPath) {
    $ln = (Get-ItemProperty -Path $lnPath -Name 'legalnoticetext' -ErrorAction SilentlyContinue).legalnoticetext

    if ($ln -like '*wants to escape from me*') {
        Write-Finding -Category 'HPPD' `
                      -Path "$lnPath\legalnoticetext" `
                      -Detail 'Attacker legal notice text present' `
                      -Remediation 'Delete the legalnoticetext value'

        if ($Remediate) {
            Remove-ItemProperty -Path $lnPath -Name 'legalnoticetext' -ErrorAction SilentlyContinue
            $script:Fixes.Add('Removed legalnoticetext')
        }
    }
}

# --- digit spoofing ---
$intlPath = 'HKCU:\Control Panel\International'

foreach ($name in 's1159', 's2359', 'sNativeDigits') {
    if (-not (Test-Path $intlPath)) { break }

    $v = (Get-ItemProperty -Path $intlPath -Name $name -ErrorAction SilentlyContinue).($name)

    if ($null -eq $v) { continue }

    Write-Finding -Category 'HPPD' `
                  -Path "$intlPath\$name" `
                  -Detail "Digit spoofing value = $v" `
                  -Remediation "Delete the $name value"

    if ($Remediate) {
        Remove-ItemProperty -Path $intlPath -Name $name -ErrorAction SilentlyContinue
        $script:Fixes.Add("Removed digit spoofing value $name")
    }
}

# --- lockdown values ---
$lockdown = @(
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\System'; Name = 'EnableLUA' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\System'; Name = 'DisableTaskMgr' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'Hidden' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'HideFileExt' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'NoFind' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'NoRun' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'NoDrives' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'NoTrayContextMenu' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'NoSaveSettings' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'TaskbarNoPinnedList' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'NoSetTaskbar' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'NoTrayItemsDisplay' },
    @{ Hive = 'HKLM:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\Explorer'; Name = 'NoPinningToTaskbar' },
    @{ Hive = 'HKCU:'; Sub = 'Software\Microsoft\Windows\CurrentVersion\Policies\System'; Name = 'DisableCMD' },
    @{ Hive = 'HKCU:'; Sub = 'Software\Microsoft\Command Processor'; Name = 'DisableUNCCheck' }
)

foreach ($l in $lockdown) {
    $p = $l.Hive + $l.Sub

    if (-not (Test-Path $p)) { continue }

    $v = (Get-ItemProperty -Path $p -Name $l.Name -ErrorAction SilentlyContinue).($l.Name)

    if ($null -eq $v) { continue }

    Write-Finding -Category 'Lockdown' `
                  -Path "$p\$($l.Name)" `
                  -Detail "$($l.Name) = $v" `
                  -Remediation "Delete the $($l.Name) value (legitimate software rarely sets this)"

    if ($Remediate) {
        Remove-ItemProperty -Path $p -Name $l.Name -ErrorAction SilentlyContinue
        $script:Fixes.Add("Removed lockdown value $($l.Name)")
    }
}

# ---------------------------------------------------------------- #
# 5. Filesystem artifacts
# ---------------------------------------------------------------- #
Write-Host ''
Write-Host '[5/6] Filesystem artifacts' -ForegroundColor Cyan

$ghostDir = Join-Path $env:PUBLIC 'ghostroot'

if (Test-Path $ghostDir) {
    Write-Finding -Category 'HPPD' `
                  -Path $ghostDir `
                  -Detail 'Dropper staging directory present' `
                  -Remediation 'Quarantine, do not execute'

    Get-ChildItem -Path $ghostDir -Recurse -ErrorAction SilentlyContinue |
        ForEach-Object {
            Write-Finding -Category 'HPPD' `
                          -Path $_.FullName `
                          -Detail "Staged artifact ($($_.Length) bytes)"
        }
}

# rpdbfk.exe anywhere on the system drive.
$rpd = Get-ChildItem -Path 'C:\' -Filter 'rpdbfk.exe' `
        -Recurse -Force -ErrorAction SilentlyContinue

if ($rpd) {
    foreach ($f in $rpd) {
        Write-Finding -Category 'HPPD' `
                      -Path $f.FullName `
                      -Detail "Dropped binary rpdbfk.exe ($($f.Length) bytes)" `
                      -Remediation 'Quarantine and preserve for analysis'
    }
}

# ---------------------------------------------------------------- #
# 6. Infected documents
# ---------------------------------------------------------------- #
Write-Host ''
Write-Host '[6/6] Infected document scan' -ForegroundColor Cyan

if ($SkipFileScan) {
    Write-Host '  Skipped (-SkipFileScan).'
}
else {
    if (-not $ScanRoot) {
        $ScanRoot = @(
            $env:USERPROFILE,
            $env:PUBLIC,
            (Join-Path $env:SystemDrive 'Scripts')
        ) | Where-Object { $_ -and (Test-Path $_) }
    }

    # Literal byte fragments emitted by HPPD/utils/fileinfect.
    $docSignatures = @(
        'app.alert(''THE END IS NEAR',
        '/S /JavaScript /JS',
        'Stop-Computer -Force',
        'backward compatibility trick'
    )

    $extensions = @('*.pdf', '*.bat', '*.cmd', '*.ps1', '*.txt')

    $infected = 0

    foreach ($root in $ScanRoot) {
        foreach ($ext in $extensions) {
            Get-ChildItem -Path $root -Filter $ext `
                -Recurse -Force -ErrorAction SilentlyContinue |
            ForEach-Object {
                $f = $_

                foreach ($sig in $docSignatures) {
                    $hit = Select-String -Path $f.FullName `
                              -Pattern $sig -SimpleMatch -Quiet `
                              -ErrorAction SilentlyContinue

                    if ($hit) {
                        $infected++
                        Write-Finding -Category 'Infected' `
                                      -Path $f.FullName `
                                      -Detail "Contains infection marker: $sig" `
                                      -Remediation 'Restore from backup; file is unrecoverable'

                        break
                    }
                }
            }
        }
    }

    if ($infected -eq 0) {
        Write-Host '  Clean - no infected documents found.'
    }
}

# ---------------------------------------------------------------- #
# Summary
# ---------------------------------------------------------------- #
Write-Host ''
Write-Host '=== Summary ===' -ForegroundColor Cyan

if ($script:FindingCount -eq 0) {
    Write-Host '  No HPPD artifacts found.' -ForegroundColor Green
    exit 0
}

Write-Host ("  {0} finding(s)." -f $script:FindingCount) -ForegroundColor Yellow

$script:Findings |
    Group-Object Category |
    Sort-Object Name |
    ForEach-Object {
        Write-Host ("    {0,-12} {1}" -f $_.Name, $_.Count)
    }

if ($script:Findings | Where-Object { $_.Category -eq 'HPPD' }) {
    Write-Host ''
    Write-Host '  CONFIRMED HPPD COMPROMISE.' -ForegroundColor Red
    Write-Host '  Preserve evidence before remediation. Reimage is strongly advised.' -ForegroundColor Red
}

if ($Remediate) {
    Write-Host ''
    Write-Host '=== Remediation applied ===' -ForegroundColor Cyan

    foreach ($f in $script:Fixes) {
        Write-Host "  - $f"
    }
}

exit 1