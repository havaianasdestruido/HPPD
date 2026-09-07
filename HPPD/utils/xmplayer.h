#pragma once

#include <windows.h>

// ============================================================
// XM TRACKER PLAYER
//
// Plays an embedded .xm module (HPPD/audio/song.xm, linked in
// as an RCDATA resource) through waveOut at 44.1 kHz 16-bit
// stereo. chibi-xmplay (HPPD/utils/xmplay) decodes and mixes
// the module; the XM bytes are read straight from the resource
// through a memory-backed XM_FileIO.
//
// Usage (same shape as the Bytebeats API):
//   XmPlayer::Reset();
//   HANDLE h = StartEffect(XmPlayer::PlayThread);
//   ...
//   XmPlayer::StopAll();
//   WaitEffect(h);
//
// ESC and the HPPD STOP latch also stop the music.
// ============================================================

namespace XmPlayer
{
    DWORD WINAPI PlayThread(LPVOID lpvd);

    void Reset();
    void StopAll();
}