#include "compress.h"


std::vector<uint8_t> compressBuffer(uint8_t* input_ptr, uint32_t input_size)
{
    z_stream zs{};
    deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED,
                 15 + 16, 8, Z_DEFAULT_STRATEGY);

    zs.next_in = reinterpret_cast<Bytef*>(input_ptr);
    zs.avail_in = input_size;

    std::vector<uint8_t> out;
    constexpr size_t CHUNK = 32 * 1024;
    uint8_t buffer[CHUNK];

    int ret;

    do {
        zs.next_out = buffer;
        zs.avail_out = CHUNK;

        ret = deflate(&zs, Z_FINISH);

        size_t have = CHUNK - zs.avail_out;
        out.insert(out.end(), buffer, buffer + have);

    } while (ret == Z_OK);

    deflateEnd(&zs);

    return out;
}