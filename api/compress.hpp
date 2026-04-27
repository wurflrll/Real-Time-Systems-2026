

#include <zlib.h>

#include <iostream>
#include <vector>


std::vector<unsigned char> gzipCompress(const std::vector<unsigned char>& input)
{
    z_stream zs{};
    deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED,
                 15 + 16, 8, Z_DEFAULT_STRATEGY);

    zs.next_in = const_cast<Bytef*>(input.data());
    zs.avail_in = input.size();

    std::vector<unsigned char> out;
    out.resize(input.size() * 1.1 + 100);

    zs.next_out = out.data();
    zs.avail_out = out.size();

    deflate(&zs, Z_FINISH);
    deflateEnd(&zs);

    out.resize(zs.total_out);
    return out;
}