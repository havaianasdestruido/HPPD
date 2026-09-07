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
// WaveOut-based 8-bit mono bytebeat. The expression is a tiny
// C-like uint32 formula over t, parsed once at Play() time:
//
//   t*(t^t>>10)|t>>2|t>>8|t&t>>4     (default)
//
// Grammar (loosest to tightest):
//   |    ^    &    << >>    + -    * / %    unary - ~ !    ()
//
// The buffer is one "loop segment" (~31 s), regenerated forever
// until a stop is requested (ESC, STOP latch, or StopAll).
// ============================================================

namespace Bytebeats
{
    const DWORD kSeconds = 31;

    namespace
    {
        volatile bool gStop = false;
        DWORD gRate = 2000;

        enum Op
        {
            OP_CONST,
            OP_T,
            OP_ADD,
            OP_SUB,
            OP_MUL,
            OP_DIV,
            OP_MOD,
            OP_AND,
            OP_OR,
            OP_XOR,
            OP_SHL,
            OP_SHR,
            OP_NEG,
            OP_NOT,
            OP_COMPL
        };

        struct Node
        {
            Op op;
            DWORD val;
            Node* l;
            Node* r;
        };

        Node* gAst = nullptr;

        static Node* NewNode(Op op, Node* l, Node* r, DWORD val = 0)
        {
            Node* n = new Node;

            n->op = op;
            n->l = l;
            n->r = r;
            n->val = val;

            return n;
        }

        static void FreeNode(Node* n)
        {
            if (!n)
                return;

            FreeNode(n->l);
            FreeNode(n->r);

            delete n;
        }

        // ----------------------------------------------------
        // Recursive descent parser.
        // ----------------------------------------------------

        static const char* SkipWs(const char* p)
        {
            while (*p == ' ' || *p == '\t')
                ++p;

            return p;
        }

        static Node* ParseOr(const char*& p);

        static Node* ParsePrimary(const char*& p)
        {
            p = SkipWs(p);

            if (*p == '(')
            {
                ++p;

                Node* n = ParseOr(p);

                p = SkipWs(p);

                if (*p == ')')
                    ++p;

                return n;
            }

            if (*p == 't')
            {
                ++p;
                return NewNode(OP_T, nullptr, nullptr);
            }

            if (*p >= '0' && *p <= '9')
            {
                DWORD v = 0;

                if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
                {
                    p += 2;

                    while (true)
                    {
                        char c = *p;

                        DWORD d;

                        if (c >= '0' && c <= '9')
                            d = (DWORD)(c - '0');
                        else if (c >= 'a' && c <= 'f')
                            d = (DWORD)(c - 'a') + 10;
                        else if (c >= 'A' && c <= 'F')
                            d = (DWORD)(c - 'A') + 10;
                        else
                            break;

                        v = v * 16 + d;
                        ++p;
                    }
                }
                else
                {
                    while (*p >= '0' && *p <= '9')
                    {
                        v = v * 10 + (DWORD)(*p - '0');
                        ++p;
                    }
                }

                return NewNode(OP_CONST, nullptr, nullptr, v);
            }

            return nullptr;
        }

        static Node* ParseUnary(const char*& p)
        {
            p = SkipWs(p);

            switch (*p)
            {
                case '-':
                {
                    ++p;

                    Node* c = ParseUnary(p);

                    if (!c)
                        return nullptr;

                    return NewNode(OP_NEG, c, nullptr);
                }
                case '~':
                {
                    ++p;

                    Node* c = ParseUnary(p);

                    if (!c)
                        return nullptr;

                    return NewNode(OP_COMPL, c, nullptr);
                }
                case '!':
                {
                    ++p;

                    Node* c = ParseUnary(p);

                    if (!c)
                        return nullptr;

                    return NewNode(OP_NOT, c, nullptr);
                }
                default:
                    return ParsePrimary(p);
            }
        }

        static bool IsMulOp(char c)
        {
            return c == '*' || c == '/' || c == '%';
        }

        static bool IsAddOp(char c)
        {
            return c == '+' || c == '-';
        }

        static bool IsShiftOp(const char*& p)
        {
            if (p[0] != '<' && p[0] != '>')
                return false;

            if (p[1] != p[0])
                return false;

            return true;
        }

        static Node* ParseMul(const char*& p)
        {
            Node* l = ParseUnary(p);

            while (true)
            {
                p = SkipWs(p);

                if (!IsMulOp(*p))
                    break;

                Op op;

                switch (*p)
                {
                    case '*': op = OP_MUL; break;
                    case '/': op = OP_DIV; break;
                    default:  op = OP_MOD; break;
                }

                ++p;

                Node* r = ParseUnary(p);

                if (!r)
                {
                    FreeNode(l);
                    return nullptr;
                }

                l = NewNode(op, l, r);
            }

            return l;
        }

        static Node* ParseAdd(const char*& p)
        {
            Node* l = ParseMul(p);

            while (true)
            {
                p = SkipWs(p);

                if (!IsAddOp(*p))
                    break;

                Op op = (*p == '+') ? OP_ADD : OP_SUB;

                ++p;

                Node* r = ParseMul(p);

                if (!r)
                {
                    FreeNode(l);
                    return nullptr;
                }

                l = NewNode(op, l, r);
            }

            return l;
        }

        static Node* ParseShift(const char*& p)
        {
            Node* l = ParseAdd(p);

            while (true)
            {
                p = SkipWs(p);

                if (!IsShiftOp(p))
                    break;

                Op op = (p[0] == '<') ? OP_SHL : OP_SHR;

                p += 2;

                Node* r = ParseAdd(p);

                if (!r)
                {
                    FreeNode(l);
                    return nullptr;
                }

                l = NewNode(op, l, r);
            }

            return l;
        }

        static Node* ParseAnd(const char*& p)
        {
            Node* l = ParseShift(p);

            while (true)
            {
                p = SkipWs(p);

                if (*p != '&' || p[1] == '&')
                    break;

                ++p;

                Node* r = ParseShift(p);

                if (!r)
                {
                    FreeNode(l);
                    return nullptr;
                }

                l = NewNode(OP_AND, l, r);
            }

            return l;
        }

        static Node* ParseXor(const char*& p)
        {
            Node* l = ParseAnd(p);

            while (true)
            {
                p = SkipWs(p);

                if (*p != '^')
                    break;

                ++p;

                Node* r = ParseAnd(p);

                if (!r)
                {
                    FreeNode(l);
                    return nullptr;
                }

                l = NewNode(OP_XOR, l, r);
            }

            return l;
        }

        static Node* ParseOr(const char*& p)
        {
            Node* l = ParseXor(p);

            while (true)
            {
                p = SkipWs(p);

                if (*p != '|' || p[1] == '|')
                    break;

                ++p;

                Node* r = ParseXor(p);

                if (!r)
                {
                    FreeNode(l);
                    return nullptr;
                }

                l = NewNode(OP_OR, l, r);
            }

            return l;
        }

        static DWORD Eval(const Node* n, DWORD t)
        {
            if (!n)
                return 0;

            switch (n->op)
            {
                case OP_CONST: return n->val;
                case OP_T:     return t;

                case OP_ADD:
                    return Eval(n->l, t) + Eval(n->r, t);
                case OP_SUB:
                    return Eval(n->l, t) - Eval(n->r, t);
                case OP_MUL:
                    return Eval(n->l, t) * Eval(n->r, t);
                case OP_DIV:
                {
                    DWORD d = Eval(n->r, t);
                    return d ? Eval(n->l, t) / d : 0;
                }
                case OP_MOD:
                {
                    DWORD d = Eval(n->r, t);
                    return d ? Eval(n->l, t) % d : 0;
                }
                case OP_AND:
                    return Eval(n->l, t) & Eval(n->r, t);
                case OP_OR:
                    return Eval(n->l, t) | Eval(n->r, t);
                case OP_XOR:
                    return Eval(n->l, t) ^ Eval(n->r, t);
                case OP_SHL:
                    return Eval(n->l, t) << (Eval(n->r, t) & 31);
                case OP_SHR:
                    return Eval(n->l, t) >> (Eval(n->r, t) & 31);

                case OP_NEG:  return 0u - Eval(n->l, t);
                case OP_NOT:  return Eval(n->l, t) ? 0u : 1u;
                case OP_COMPL: return ~Eval(n->l, t);
            }

            return 0;
        }

        static bool StopRequested()
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

    bool Play(const char* expr, DWORD rate)
    {
        FreeNode(gAst);
        gAst = nullptr;

        if (!expr)
            return false;

        const char* p = expr;

        Node* n = ParseOr(p);

        p = SkipWs(p);

        if (!n || *p != '\0')
        {
            FreeNode(n);
            return false;
        }

        gAst = n;
        gRate = (rate > 0) ? rate : 2000;

        return true;
    }

    DWORD WINAPI BytebeatPlay(LPVOID lpvd)
    {
        HWAVEOUT hWaveOut = 0;
        WAVEFORMATEX wfx = {
            WAVE_FORMAT_PCM, 1, gRate, gRate, 1, 8, 0
        };

        if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
            return 1;

        const DWORD kSamples = gRate * kSeconds;

        BYTE* sbuffer = new BYTE[kSamples];

        while (!StopRequested())
        {
            for (DWORD t = 0; t < kSamples; ++t)
            {
                DWORD v = gAst
                    ? Eval(gAst, t)
                    : t * (t ^ t >> 10) | t >> 2 | t >> 8 | t & t >> 4;

                sbuffer[t] = (BYTE)(v >> 8);
            }

            WAVEHDR header = {
                (LPSTR)sbuffer, kSamples, 0, 0, 0, 0, 0, 0
            };

            waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
            waveOutSetVolume(hWaveOut, MAKELONG(0xFFFF, 0xFFFF));
            waveOutUnprepareHeader(hWaveOut, &header, sizeof(header));

            for (DWORD i = 0; i < kSeconds && !StopRequested(); ++i)
                Sleep(1000);
        }

        waveOutReset(hWaveOut);
        waveOutClose(hWaveOut);

        delete[] sbuffer;

        return 0x00;
    }
}