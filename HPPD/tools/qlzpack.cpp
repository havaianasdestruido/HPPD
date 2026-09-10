// qlzpack - QuickLZ 1.5.0 one-shot file compressor.
//
// Used at build time to compress audio/song.xm into the RCDATA
// blob audio/song.xm.qlz that utils/xmplayer.cpp decompresses
// at runtime (level 1, streaming disabled - see README).
//
// Usage:
//   qlzpack <infile> <outfile>
//
// Build (VS dev prompt, x64/x86 both fine - output is portable):
//   cl /nologo /O2 -DQLZ_COMPRESSION_LEVEL=1 -DQLZ_STREAMING_BUFFER=0 qlzpack.cpp

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

extern "C" {
#include "../../third_party/quicklz/quicklz.h"
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::fprintf(stderr, "usage: qlzpack <infile> <outfile>\n");
        return 2;
    }

    FILE* in = std::fopen(argv[1], "rb");
    if (!in)
    {
        std::fprintf(stderr, "qlzpack: cannot open %s\n", argv[1]);
        return 1;
    }

    std::fseek(in, 0, SEEK_END);
    const long fileSize = std::ftell(in);
    std::fseek(in, 0, SEEK_SET);

    if (fileSize <= 0)
    {
        std::fprintf(stderr, "qlzpack: empty input\n");
        std::fclose(in);
        return 1;
    }

    std::vector<char> src(static_cast<size_t>(fileSize));

    if (std::fread(src.data(), 1, static_cast<size_t>(fileSize), in) !=
        static_cast<size_t>(fileSize))
    {
        std::fprintf(stderr, "qlzpack: short read\n");
        std::fclose(in);
        return 1;
    }

    std::fclose(in);

    // Level 1, streaming disabled: the state can be reused, the
    // hash table just needs to start zeroed.
    qlz_state_compress state;
    std::memset(&state, 0, sizeof(state));

    // QuickLZ requires the destination to be at least
    // source + 400 bytes.
    const size_t bound =
        src.size() + 400;

    std::vector<char> dst(bound);

    const size_t packed =
        qlz_compress(
            src.data(),
            dst.data(),
            src.size(),
            &state
        );

    if (packed == 0 || packed > bound)
    {
        std::fprintf(stderr, "qlzpack: compression failed\n");
        return 1;
    }

    FILE* out = std::fopen(argv[2], "wb");
    if (!out)
    {
        std::fprintf(stderr, "qlzpack: cannot open %s\n", argv[2]);
        return 1;
    }

    if (std::fwrite(dst.data(), 1, packed, out) != packed)
    {
        std::fprintf(stderr, "qlzpack: short write\n");
        std::fclose(out);
        return 1;
    }

    std::fclose(out);

    std::printf(
        "qlzpack: %s (%ld bytes) -> %s (%zu bytes, %.1f%%)\n",
        argv[1],
        fileSize,
        argv[2],
        packed,
        100.0 * (1.0 - static_cast<double>(packed) /
            static_cast<double>(fileSize))
    );

    return 0;
}