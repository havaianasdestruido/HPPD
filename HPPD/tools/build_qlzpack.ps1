# Builds tools/qlzpack.exe - the QuickLZ 1.5.0 build-time
# compressor that turns audio/song.xm into audio/song.xm.qlz.
#
# Requires the Visual Studio C++ build tools (same install used
# for HPPD.vcxproj). Run from a "Developer Command Prompt", or
# pass the path to vcvars manually:
#
#   powershell -File build_qlzpack.ps1
#   powershell -File build_qlzpack.ps1 -Vcvars "...\vcvars64.bat"
# ------------------------------------------------------------
param(
    [string]$Vcvars = ""
)

$ErrorActionPreference = "Stop"

$root = Split-Path $PSScriptRoot -Parent        # repo root
$tool  = Join-Path $PSScriptRoot "qlzpack.cpp"
$qlzC  = Join-Path $root "third_party\quicklz\quicklz.c"
$outExe = Join-Path $PSScriptRoot "qlzpack.exe"

if (-not (Test-Path $tool) -or -not (Test-Path $qlzC)) {
    Write-Error "qlzpack.cpp or third_party/quicklz/quicklz.c not found"
}

if (-not $Vcvars) {
    $candidates = @(
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
    $Vcvars = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if (-not $Vcvars) {
        Write-Error "vcvars not found - pass -Vcvars <path> or open a Developer Command Prompt"
    }
}

$cmdline = "call `"$Vcvars`" && cl /nologo /O2 /DQLZ_COMPRESSION_LEVEL=1 /DQLZ_STREAMING_BUFFER=0 `"$tool`" `"$qlzC`" /Fe:`"$outExe`""
cmd /c $cmdline
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
Write-Output "built $outExe"