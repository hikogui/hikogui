// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gzip.hpp"
#include "inflate.hpp"
#include "../utility/module.hpp"
#include "../placement.hpp"

hi_warning_push();
// C26481: Don't use pointer arithmetic. Use span instead (bounds.1).
// False positive, this file does not do any pointer arithmetic.
hi_warning_ignore_msvc(26481)

namespace hi::inline v1 {

struct GZIPMemberHeader {
    uint8_t ID1;
    uint8_t ID2;
    uint8_t CM;
    uint8_t FLG;
    little_uint32_buf_t MTIME;
    uint8_t XFL;
    uint8_t OS;
};

static bstring gzip_decompress_member(std::span<std::byte const> bytes, std::size_t &offset, std::size_t max_size)
{
    hilet header = make_placement_ptr<GZIPMemberHeader>(bytes, offset);

    hi_check(header->ID1 == 31, "GZIP Member header ID1 must be 31");
    hi_check(header->ID2 == 139, "GZIP Member header ID2 must be 139");
    hi_check(header->CM == 8, "GZIP Member header CM must be 8");
    hi_check((header->FLG & 0xe0) == 0, "GZIP Member header FLG reserved bits must be 0");
    hi_check(header->XFL == 2 or header->XFL == 4, "GZIP Member header XFL must be 2 or 4");
    [[maybe_unused]] hilet FTEXT = to_bool(header->FLG & 1);
    hilet FHCRC = to_bool(header->FLG & 2);
    hilet FEXTRA = to_bool(header->FLG & 4);
    hilet FNAME = to_bool(header->FLG & 8);
    hilet FCOMMENT = to_bool(header->FLG & 16);

    if (FEXTRA) {
        hilet XLEN = **make_placement_ptr<little_uint16_buf_t>(bytes, offset);
        offset += XLEN;
    }

    if (FNAME) {
        auto c = std::byte{};
        do {
            hi_check(offset < bytes.size(), "GZIP Member header FNAME reading beyond end of buffer");
            c = bytes[offset++];
        } while (c != std::byte{0});
    }

    if (FCOMMENT) {
        auto c = std::byte{};
        do {
            hi_check(offset < bytes.size(), "GZIP Member header FCOMMENT reading beyond end of buffer");
            c = bytes[offset++];
        } while (c != std::byte{0});
    }

    if (FHCRC) {
        [[maybe_unused]] hilet CRC16 = make_placement_ptr<little_uint16_buf_t>(bytes, offset);
    }

    auto r = inflate(bytes, offset, max_size);

    [[maybe_unused]] auto CRC32 = **make_placement_ptr<little_uint32_buf_t>(bytes, offset);
    [[maybe_unused]] auto ISIZE = **make_placement_ptr<little_uint32_buf_t>(bytes, offset);

    hi_check(
        ISIZE == (size(r) & 0xffffffff),
        "GZIP Member header ISIZE must be same as the lower 32 bits of the inflated size.");
    return r;
}

bstring gzip_decompress(std::span<std::byte const> bytes, std::size_t max_size)
{
    auto r = bstring{};

    auto offset = 0_uz;
    while (offset < bytes.size()) {
        auto member = gzip_decompress_member(bytes, offset, max_size);
        max_size -= member.size();
        r.append(member);
    }

    return r;
}

} // namespace hi::inline v1

hi_warning_pop();
