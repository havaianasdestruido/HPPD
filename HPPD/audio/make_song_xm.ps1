param(
    [string]$OutFile = (Join-Path $PSScriptRoot "song.xm")
)

# Placeholder XM generator. Layout targets the chibi-xmplay
# loader (utils\xmplay\xmplay.c) so the embedded song decodes
# exactly as written. The user's real song just replaces
# song.xm.

function Write-Ascii([System.IO.BinaryWriter]$w, [string]$s, [int]$len) {
    $b = [System.Text.Encoding]::ASCII.GetBytes($s)
    $w.Write($b)
    $pad = $len - $b.Length
    for ($i = 0; $i -lt $pad; $i++) { $w.Write([byte]0) }
}

$srate = 22050
$pluckSamples = $srate / 2        # 0.5 s
$freq = 220.0

$bw = New-Object System.IO.BinaryWriter([System.IO.File]::Create($OutFile))

# ---- module header ----
Write-Ascii $bw "Extended Module: " 17
Write-Ascii $bw "HPPD placeholder" 20
$bw.Write([byte]0x1A)
Write-Ascii $bw "chibi-xmplay/HPPD" 20
$bw.Write([uint16]0x0104)          # track format version 1.04
$bw.Write([uint32]276)             # header size (incl. order table)
$bw.Write([uint16]1)               # song length (orders)
$bw.Write([uint16]0)               # restart position
$bw.Write([uint16]1)               # channels
$bw.Write([uint16]1)               # patterns
$bw.Write([uint16]1)               # instruments
$bw.Write([uint16]0x0001)          # flags: linear periods
$bw.Write([uint16]6)               # default speed
$bw.Write([uint16]125)             # default tempo
$bw.Write([byte]0)                 # order 0 -> pattern 0
for ($i = 1; $i -lt 256; $i++) { $bw.Write([byte]0xFF) }

# ---- pattern 0 (pre-packed, chibi internal format) ----
# first byte rows-1, then events; 0xE0+(rows-1) advances rows,
# 0x43 = note+instrument, then note, instrument, 0x60 = end.
$pat = New-Object System.Collections.Generic.List[byte]
$pat.Add([byte]63)                       # rows - 1
$pat.Add([byte]0x00)                     # seek channel 0, row 0
$pat.Add([byte]0x43); $pat.Add([byte]49); $pat.Add([byte]0x00) # C-4
foreach ($n in @(53, 56, 61, 56, 53, 49, 56)) { # E4 G4 C5 G4 E4 C4 G4
    $pat.Add([byte]0xE7)                  # advance 8 rows
    $pat.Add([byte]0x43); $pat.Add([byte]$n); $pat.Add([byte]0x00)
}
$pat.Add([byte]0x60)

$bw.Write([uint32]9)                     # pattern header length (chibi layout, no pad)
$bw.Write([byte]1)                       # packing: pre-packed
$bw.Write([uint16]64)                    # rows
$bw.Write([uint16]$pat.Count)            # packed data size
$bw.Write($pat.ToArray())                # data immediately after the 9-byte header

# ---- instrument 0 ----
$bw.Write([uint32]243)                   # instrument header size
Write-Ascii $bw "Sine pluck" 22
$bw.Write([byte]0)                       # type
$bw.Write([uint16]1)                     # samples (word, chibi layout)
$bw.Write([uint32]40)                    # sample header size
for ($i = 0; $i -lt 96; $i++) { $bw.Write([byte]0) } # note map
# volume envelope: 12 x (u16 offset, u16 value)
$bw.Write([uint16]0);    $bw.Write([uint16]64)   # point 0 (64)
$bw.Write([uint16]8);    $bw.Write([uint16]0)    # point 1 (decay)
for ($i = 2; $i -lt 12; $i++) { $bw.Write([uint16]0); $bw.Write([uint16]0) }
for ($i = 0; $i -lt 24; $i++) { $bw.Write([uint16]0) } # pan envelope
$bw.Write([byte]0x82)                    # 2 env points + enabled
$bw.Write([byte]0x00)                    # pan env flags
$bw.Write([byte]0); $bw.Write([byte]0); $bw.Write([byte]0)
$bw.Write([byte]0); $bw.Write([byte]0); $bw.Write([byte]0)
$bw.Write([byte]0x01)                    # volume type: envelope on
$bw.Write([byte]0x00)                    # pan type
$bw.Write([byte]0); $bw.Write([byte]0); $bw.Write([byte]0); $bw.Write([byte]0) # vibrato
$bw.Write([uint16]0)                     # fadeout
$bw.Write([uint16]0)                     # reserved

# ---- sample header (40 bytes) ----
$bw.Write([uint32]$pluckSamples)         # length (bytes)
$bw.Write([uint32]0)                     # loop start
$bw.Write([uint32]0)                     # loop end
$bw.Write([byte]64)                      # volume
$bw.Write([byte]0)                       # finetune
$bw.Write([byte]0x10)                    # PCM16, no loop
$bw.Write([byte]128)                     # pan center
$bw.Write([byte]49)                      # base note C-4
$bw.Write([byte]0)                       # reserved
Write-Ascii $bw "HPPD sine pluck" 22

# ---- sample data: decaying sine ----
$pi = [Math]::PI
for ($i = 0; $i -lt $pluckSamples; $i++) {
    $t = $i / $srate
    $v = [Math]::Sin(2.0 * $pi * $freq * $t) * [Math]::Exp(-6.0 * $t)
    $s = [int]($v * 8000.0)
    if ($s -gt 32767) { $s = 32767 } elseif ($s -lt -32768) { $s = -32768 }
    $bw.Write([int16]$s)
}

$bw.Flush()
$bw.Close()
Write-Output "wrote $OutFile ($([System.IO.File]::Exists($OutFile)) / $((Get-Item $OutFile).Length) bytes)"