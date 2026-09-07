#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#include "bytebeats.h"

extern volatile bool gFxStop; // defined in payloads.cpp

// ============================================================
// BYTEBEAT SOUND
//
// WaveOut-based 8-bit mono bytebeat at 2000 Hz:
//
//   t*(t^t>>10)|t>>2|t>>8|t&t>>4
//
// The buffer is one "loop segment" (~31 s), regenerated forever
// until a stop is requested (ESC, STOP latch, or StopAll).
// ============================================================

namespace Bytebeats
{
    const DWORD kSampleRate = 2000;
    const DWORD kSeconds    = 31;
    const DWORD kSamples    = kSampleRate * kSeconds;

    namespace
    {
        volatile bool gStop = false;

        static inline bool StopRequested()
        {
            return gStop || gFxStop ||
                (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        }
    }

    void Reset()
    {
        gStop = false;
    }

    void StopAll()
    {
        gStop = true;
    }

    DWORD WINAPI BytebeatPlay(LPVOID lpvd)
    {
        HWAVEOUT hWaveOut = 0;
        WAVEFORMATEX wfx = {
            WAVE_FORMAT_PCM, 1, kSampleRate, kSampleRate, 1, 8, 0
        };

        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
            return 1;

        BYTE* sbuffer = new BYTE[kSamples];

        while (!StopRequested())
        {
            for (DWORD t = 0; t < kSamples; ++t)
            {
                DWORD v = t * (t ^ t >> 10) | t >> 2 | t >> 8 | t & t >> 4;

                sbuffer[t] = (BYTE)(v >> 8);
            }

            WAVEHDR header = {
                (LPSTR)sbuffer, kSamples, 0, 0, 0, 0, 0, 0
            };

            waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutSetVolume(hWaveOut, MAKELONG(0xFFFF, 0xFFFF));
            waveOutUnprepareHeader(hWaveOut, &header, sizeof(header));

            for (int i = 0; i < kSeconds && !StopRequested(); ++i)
                Sleep(1000);
        }

        waveOutReset(hWaveOut);
        waveOutClose(hWaveOut);

        delete[] sbuffer;

        return 0x00;
    }
}