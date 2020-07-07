// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/foundation/zlib.hpp"
#include "ttauri/foundation/inflate.hpp"
#include "ttauri/foundation/endian.hpp"
#include "ttauri/foundation/placement.hpp"

namespace tt {

struct zlib_header {
    uint8_t CMF;
    uint8_t FLG;
};

bstring zlib_decompress(nonstd::span<std::byte const> bytes, ssize_t max_size)
{
    ssize_t offset = 0;

    ttlet header = make_placement_ptr<zlib_header>(bytes, offset);

    ttlet header_chksum = header->CMF * 256 + header->FLG;
    parse_assert2(header_chksum % 31 == 0, "zlib header checksum failed.");

    parse_assert2((header->CMF & 0xf) == 8, "zlib compression method must be 8");
    parse_assert2(((header->CMF >> 4) & 0xf) <= 7, "zlib LZ77 window too large");
    parse_assert2((header->FLG & 0x20) == 0, "zlib must not use a preset dicationary");

    if (header->FLG & 0x20) {
        [[maybe_unused]] auto FDICT = make_placement_ptr<big_uint32_buf_t>(bytes, offset);
    }

    auto r = inflate(bytes, offset, max_size);

    [[maybe_unused]] auto ADLER32 = make_placement_ptr<big_uint32_buf_t>(bytes, offset);

    return r;
}


}