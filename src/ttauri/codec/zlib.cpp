// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "zlib.hpp"
#include "inflate.hpp"
#include "../endian.hpp"
#include "../placement.hpp"

namespace tt::inline v1 {

struct zlib_header {
    uint8_t CMF;
    uint8_t FLG;
};

bstring zlib_decompress(std::span<std::byte const> bytes, ssize_t max_size)
{
    ssize_t offset = 0;

    ttlet header = make_placement_ptr<zlib_header>(bytes, offset);

    ttlet header_chksum = header->CMF * 256 + header->FLG;
    tt_parse_check(header_chksum % 31 == 0, "zlib header checksum failed.");

    tt_parse_check((header->CMF & 0xf) == 8, "zlib compression method must be 8");
    tt_parse_check(((header->CMF >> 4) & 0xf) <= 7, "zlib LZ77 window too large");
    tt_parse_check((header->FLG & 0x20) == 0, "zlib must not use a preset dicationary");

    if (header->FLG & 0x20) {
        [[maybe_unused]] auto FDICT = make_placement_ptr<big_uint32_buf_t>(bytes, offset);
    }

    auto r = inflate(bytes, offset, max_size);

    [[maybe_unused]] auto ADLER32 = make_placement_ptr<big_uint32_buf_t>(bytes, offset);

    return r;
}

} // namespace tt::inline v1