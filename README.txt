HPPD (a.k.a. Hallucinogen Persisting Perception Disorder)
-------------------------------------------------------

Hallucinogen persisting perception disorder (HPPD) is a
rare medical condition where a person experiences
ongoing or repeating visual disturbances and perceptual
distortions long after the effects of a psychedelic drug
have worn off.

---

This is (probably) my last GDI trojan. Yes, I know that the
"GDI trojan scene" is pretty much dead now, but I decided to
make my last demo right now, in 2026

-------------------------------------------------------

!!! IMPORTANT NOTES !!!

Running on Windows XP requires OneCoreAPI to bypass the
"GetTickCount64" error produced by Kernel32.

(AFAIK it is due to the visual payload time indicator)

I used "Windows 7" and it worked flawlessly.

---

If you have troubles on Windows XP with DLLs, use this:
https://aka.ms/vs/15/release/vc_redist.x86.exe (32-bit)
https://aka.ms/vs/15/release/vc_redist.x64.exe (64-bit)

-------------------------------------------------------

TYPE:..... destructive (but not 100% yet)
LANGUAGES: C++,
	       C (headers),
           PowerShell (scripts for
                       compressing
                       the XM song
                       used)
PLATFORM:. WIN32 (x86 but supports x64)
TESTED ON: Windows XP SP3 32-bit (VM), [1]
	       Windows 10 64-bit (Main PC)
FEATURES:. Pretty much fits every kind
           of malicious program,
           except for Remote Controling
           and server-sided behavior.

           I can think of:
           Persistent;
           (very hard to clean its
           destructible system payloads)
           Trojan (?);
           System Hijacker;
           (messes up with registry)
           Browser Hijacker;
           (messes up with IE)
           TBD...

[1]: check "!!! IMPORTANT NOTES !!!" for running on
WXP.

-------------------------------------------------------

THANKS TO:

BZL, Crome, Dualtrax, Dubmood, Funky Fish, Nagz, Seablue
Sinny, Svenzzon and zabutom
For making "The Megatune", the XM song i've used.
(https://demozoo.org/music/102821/)

Pankoza - I modded some of his payloads from his Bromine
trojan (https://github.com/pankoza2-pl/BromineTrojan)

ArTicZera - ditto, but on Lixo
(https://github.com/ArTicZera/GDI-Trojan.Win32.Lixo)

Other libs are credited on the source
code, and everyone listed here and on the
full source will be credited on the MBR
once I finish everything

-------------------------------------------------------

LICENSE

Apache 2.0

-------------------------------------------------------

QUICKLZ (build-time dependency)

The embedded soundtrack (audio/song.xm) is shipped inside the
executable as a QuickLZ-compressed blob (audio/song.xm.qlz) to
shrink the binary. Building HPPD requires the QuickLZ 1.5.0 C
sources, which are NOT bundled with this repo:

  - Version: QuickLZ 1.5.0 (final, 2011)
  - Where it must be placed:
      third_party/quicklz/quicklz.c
      third_party/quicklz/quicklz.h
  - Official source: http://www.quicklz.com/
    (or any mirror shipping the "1.5.0 final" .c/.h pair;
     the files verify each other via qlz_version checks)

Compile settings: QLZ_COMPRESSION_LEVEL=1 and
QLZ_STREAMING_BUFFER=0 (already wired into HPPD.vcxproj and
tools/build_qlzpack.ps1).

Build-time flow:

  1. tools\build_qlzpack.ps1   - compiles tools\qlzpack.exe
  2. audio\make_song_xm.ps1    - regenerates the placeholder
                                 song.xm AND recompresses it
                                 into song.xm.qlz

To ship a different song, drop it as audio\song.xm and rerun
qlzpack (or make_song_xm.ps1) to refresh audio\song.xm.qlz
before building - song.rc embeds that .qlz file.

Runtime: utils\xmplayer.cpp decompresses the blob with
qlz_decompress() before feeding it to the chibi-xmplay loader.

NOTE: QuickLZ 1.5.0 is GPL (dual-licensed commercially by
quicklz.com); the bundled copy lives under third_party\quicklz
with its own LICENSE and is not covered by this project's Apache
2.0 terms.
