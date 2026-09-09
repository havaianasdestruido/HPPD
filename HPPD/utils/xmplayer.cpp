#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <mmsystem.h>
#include <malloc.h>

#pragma comment(lib, "winmm.lib")

#include "xmplayer.h"

extern "C" {
#include "xmplay/xmplay.h"
}
#include "../audio/resource.h"

extern volatile bool gFxStop; // defined in payloads.cpp

namespace XmPlayer
{
    const DWORD kSampleRate      = 44100;
    const DWORD kChannels        = 2;
    const DWORD kBytesPerSample  = 2;
    const DWORD kFramesPerBuffer = 4096;
    const int   kNumBuffers      = 3;
    const int   kOutputShift     = 16;

    namespace
    {
        volatile bool gStop = false;

        static inline bool StopRequested()
        {
            return gStop || gFxStop ||
                (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        }

        // ----------------------------------------------------
        // Memory manager for chibi-xmplay (single instance).
        // ----------------------------------------------------
        static void* MgAlloc(xm_u32 size, XM_MemoryAllocType /*type*/)
        {
            return malloc(size);
        }

        static void MgFree(void* p, XM_MemoryAllocType /*type*/)
        {
            free(p);
        }

        static XM_MemoryManager gMem = { MgAlloc, MgFree };

        // ----------------------------------------------------
        // Memory-backed FileIO: chibi reads the module straight
        // from the embedded RCDATA blob.
        // ----------------------------------------------------
        static const unsigned char* gMemData  = nullptr;
        static DWORD gMemSize  = 0;
        static DWORD gMemPos   = 0;
        static bool  gMemInUse = false;

        static xm_bool FioEof()
        {
            return gMemPos >= gMemSize ? xm_true : xm_false;
        }

        static xm_bool FioInUse()
        {
            return gMemInUse ? xm_true : xm_false;
        }

        static XM_FileIOError FioOpen(const char* /*p_file*/, xm_bool /*p_big_endian*/)
        {
            if (!gMemData || gMemSize == 0)
                return XM_FILE_ERROR_CANT_OPEN;

            gMemPos   = 0;
            gMemInUse = true;

            return XM_FILE_OK;
        }

        static xm_u8 FioGetU8()
        {
            return gMemPos < gMemSize ? gMemData[gMemPos++] : 0;
        }

        static xm_u16 FioGetU16()
        {
            return (xm_u16)(FioGetU8() | (FioGetU8() << 8));
        }

        static xm_u32 FioGetU32()
        {
            xm_u32 v = FioGetU8();
            v |= (xm_u32)FioGetU8() << 8;
            v |= (xm_u32)FioGetU8() << 16;
            v |= (xm_u32)FioGetU8() << 24;
            return v;
        }

        static void FioGetBytes(xm_u8* dst, xm_u32 count)
        {
            for (xm_u32 i = 0; i < count; ++i)
                *dst++ = FioGetU8();
        }

        static void FioSeek(xm_u32 offset)
        {
            gMemPos = offset;
        }

        static xm_u32 FioTell()
        {
            return gMemPos;
        }

        static void FioClose()
        {
            gMemInUse = false;
        }

        static XM_FileIO gFio = {
            FioEof, FioInUse, FioOpen,
            FioGetU8, FioGetU16, FioGetU32, FioGetBytes,
            FioSeek, FioTell, FioClose
        };
    }

    void Reset()
    {
        gStop = false;
    }

    void StopAll()
    {
        gStop = true;
    }

    DWORD WINAPI PlayThread(LPVOID /*lpvd*/)
    {
        HINSTANCE hInst = GetModuleHandleW(nullptr);

        HRSRC hRes =
            FindResourceW(hInst, MAKEINTRESOURCE(IDR_XM_SONG), RT_RCDATA);

        if (!hRes)
            return 1;

        HGLOBAL hGlob = LoadResource(hInst, hRes);

        if (!hGlob)
            return 1;

        void* data = LockResource(hGlob);
        DWORD size = SizeofResource(hInst, hRes);

        if (!data || size == 0)
            return 1;

        // Init order matters: chibi requires the memory manager
        // before the mixer, and the FileIO before loading.
        xm_set_memory_manager(&gMem);
        xm_create_software_mixer(kSampleRate, 16);
        xm_loader_set_fileio(&gFio);

        gMemData  = static_cast<const unsigned char*>(data);
        gMemSize  = size;
        gMemPos   = 0;
        gMemInUse = false;

        XM_Song* song = xm_song_alloc();

        if (!song)
            return 1;

        if (xm_loader_open_song("", song) != XM_LOADER_OK)
            return 1;

        xm_player_set_song(song);
        xm_player_play();

        WAVEFORMATEX wfx = {
            WAVE_FORMAT_PCM,      /* wFormatTag */
            kChannels,            /* nChannels */
            kSampleRate,          /* nSamplesPerSec */
            kSampleRate * kChannels * kBytesPerSample, /* nAvgBytesPerSec */
            kChannels * kBytesPerSample,               /* nBlockAlign */
            kBytesPerSample * 8,   /* wBitsPerSample */
            sizeof(WAVEFORMATEX)
        };

        HWAVEOUT hwo = 0;

        if (waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
            return 1;

        waveOutSetVolume(hwo, MAKELONG(0xFFFF, 0xFFFF));

        BYTE*   sbuf[kNumBuffers] = {};
        WAVEHDR hdr[kNumBuffers]  = {};
        bool    sent[kNumBuffers] = {};

        xm_s32* mixScratch =
            new xm_s32[kFramesPerBuffer * kChannels];

        const DWORD frameBytes =
            kFramesPerBuffer * kChannels * kBytesPerSample;

        for (int i = 0; i < kNumBuffers; ++i)
            sbuf[i] = new BYTE[frameBytes];

        int slot = 0;

        while (!StopRequested())
        {
            // Wait for the slot to be free (first time it never
            // was sent, every other time WHDR_DONE).
            while (sent[slot] && !(hdr[slot].dwFlags & WHDR_DONE))
                Sleep(1);

            if (sent[slot])
            {
                waveOutUnprepareHeader(hwo, &hdr[slot], sizeof(WAVEHDR));
                sent[slot] = false;
            }

            // The mixer accumulates into the scratch buffer, so
            // it must start zeroed every frame.
            ZeroMemory(mixScratch, kFramesPerBuffer * kChannels * sizeof(xm_s32));

            xm_software_mix_to_buffer(mixScratch, kFramesPerBuffer);

            // 32-bit mix -> 16-bit stereo with clamp.
            SHORT* out = reinterpret_cast<SHORT*>(sbuf[slot]);

            for (DWORD i = 0; i < kFramesPerBuffer * kChannels; ++i)
            {
                int v = mixScratch[i] >> kOutputShift;

                if (v > 32767)
                    v = 32767;
                if (v < -32768)
                    v = -32768;

                out[i] = static_cast<SHORT>(v);
            }

            ZeroMemory(&hdr[slot], sizeof(WAVEHDR));

            hdr[slot].lpData          = reinterpret_cast<LPSTR>(sbuf[slot]);
            hdr[slot].dwBufferLength  = frameBytes;

            waveOutPrepareHeader(hwo, &hdr[slot], sizeof(WAVEHDR));
            waveOutWrite(hwo, &hdr[slot], sizeof(WAVEHDR));
            sent[slot] = true;

            slot = (slot + 1) % kNumBuffers;
        }

        waveOutReset(hwo);

        for (int i = 0; i < kNumBuffers; ++i)
        {
            if (sent[i])
                waveOutUnprepareHeader(hwo, &hdr[i], sizeof(WAVEHDR));
        }

        waveOutClose(hwo);

        for (int i = 0; i < kNumBuffers; ++i)
            delete[] sbuf[i];

        delete[] mixScratch;

        xm_loader_free_song(song);
        xm_song_free(song);

        return 0;
    }
}