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

-------------------------------------------------------

TYPE:..... destructive (but not 100% yet)
LANGUAGES: C++,
	   C (headers)
PLATFORM:. WIN32 (x86 but supports x64)
TESTED ON: Windows XP SP3 32-bit (VM), [1]
	   Windows 10 64-bit (Main PC)

[1]: check "!!! IMPORTANT NOTES !!!" for running on
WXP.

-------------------------------------------------------

THANKS TO:

Pankoza - I modded some of his payloads from his Bromine
trojan (https://github.com/pankoza2-pl/BromineTrojan)

ArTicZera - ditto, but on Lixo
(https://github.com/ArTicZera/GDI-Trojan.Win32.Lixo)

-------------------------------------------------------

LICENSE

Apache 2.0