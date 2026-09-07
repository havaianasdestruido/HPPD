#ifndef UNICODE_RANDOM_H
#define UNICODE_RANDOM_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

// ============================================================
// Random Unicode string generator
// WinXP compatible
// Uses only Win32 APIs.
// ============================================================

static DWORD g_UnicodeSeed = 0;

static DWORD UnicodeRandomDWORD()
{
    if (g_UnicodeSeed == 0)
    {
        LARGE_INTEGER counter;

        if (QueryPerformanceCounter(&counter))
        {
            g_UnicodeSeed =
                (DWORD)counter.LowPart ^
                (DWORD)counter.HighPart ^
                GetTickCount() ^
                GetCurrentProcessId();
        }
        else
        {
            g_UnicodeSeed =
                GetTickCount() ^
                GetCurrentProcessId();
        }

        // Prevent a zero seed.
        if (g_UnicodeSeed == 0)
            g_UnicodeSeed = 0x12345678;
    }

    // Xorshift32
    g_UnicodeSeed ^= g_UnicodeSeed << 13;
    g_UnicodeSeed ^= g_UnicodeSeed >> 17;
    g_UnicodeSeed ^= g_UnicodeSeed << 5;

    return g_UnicodeSeed;
}

static wchar_t UnicodeRandomChar()
{
    struct UnicodeRange
    {
        WORD first;
        WORD last;
    };

    static const UnicodeRange ranges[] =
    {
        // Combining Diacritical Marks
        { 0x0300, 0x036F },

        // Greek and Coptic
        { 0x0370, 0x03FF },

        // Cyrillic
        { 0x0400, 0x04FF },

        // Hebrew
        { 0x0590, 0x05FF },

        // Arabic
        { 0x0600, 0x06FF },

        // Thai
        { 0x0E00, 0x0E7F },

        // Mathematical Operators
        { 0x2200, 0x22FF },

        // Miscellaneous Technical
        { 0x2300, 0x23FF },

        // Miscellaneous Symbols
        { 0x2600, 0x26FF },

        // Dingbats
        { 0x2700, 0x27BF },

        // CJK Unified Ideographs
        { 0x4E00, 0x9FFF },

        // Fullwidth Forms
        { 0xFF00, 0xFFEF }
    };

    const int rangeCount =
        sizeof(ranges) / sizeof(ranges[0]);

    DWORD randomValue = UnicodeRandomDWORD();

    int rangeIndex =
        (int)(randomValue % rangeCount);

    WORD first = ranges[rangeIndex].first;
    WORD last = ranges[rangeIndex].last;

    return (wchar_t)(
        first +
        (UnicodeRandomDWORD() % (last - first + 1))
        );
}

// Generates a random Unicode string.
//
// output   = destination buffer
// maxChars = maximum number of wchar_t characters INCLUDING '\0'
//
// Example:
//     wchar_t text[64];
//     GenerateRandomUnicode(text, 64);
static void GenerateRandomUnicode(
    wchar_t* output,
    int maxChars
)
{
    if (!output || maxChars <= 0)
        return;

    // Random length between 8 and 32 characters.
    int length =
        8 + (int)(UnicodeRandomDWORD() % 25);

    // Leave room for null terminator.
    if (length >= maxChars)
        length = maxChars - 1;

    int i;

    for (i = 0; i < length; ++i)
        output[i] = UnicodeRandomChar();

    output[length] = L'\0';
}

#endif // UNICODE_RANDOM_H