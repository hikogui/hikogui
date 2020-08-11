// Copyright 2020 Pokitec
// All rights reserved.

#include "gzip.hpp"
#include "inflate.hpp"
#include "../endian.hpp"
#include "../placement.hpp"

namespace tt {

struct GZIPMemberHeader {
    uint8_t ID1;
    uint8_t ID2;
    uint8_t CM;
    uint8_t FLG;
    little_uint32_buf_t MTIME;
    uint8_t XFL;
    uint8_t OS;
};

static bstring gzip_decompress_member(std::span<std::byte const> bytes, ssize_t &offset, ssize_t max_size)
{
    ttlet header = make_placement_ptr<GZIPMemberHeader>(bytes, offset);

    parse_assert(header->ID1 == 31);
    parse_assert(header->ID2 == 139);
    parse_assert(header->CM == 8);
    parse_assert((header->FLG & 0xe0) == 0); // reserved bits must be zero.
    parse_assert(header->XFL == 2 || header->XFL == 4);
    [[maybe_unused]] ttlet FTEXT = static_cast<bool>(header->FLG & 1);
    ttlet FHCRC = static_cast<bool>(header->FLG & 2);
    ttlet FEXTRA = static_cast<bool>(header->FLG & 4);
    ttlet FNAME = static_cast<bool>(header->FLG & 8);
    ttlet FCOMMENT = static_cast<bool>(header->FLG & 16);

    if (FEXTRA) {
        ttlet XLEN = make_placement_ptr<little_uint16_buf_t>(bytes, offset);
        offset += XLEN->value();
    }

    if (FNAME) {
        std::byte c;
        do {
            parse_assert(offset < std::ssize(bytes));
            c = bytes[offset++];
        } while (c != std::byte{0});
    }

    if (FCOMMENT) {
        std::byte c;
        do {
            parse_assert(offset < std::ssize(bytes));
            c = bytes[offset++];
        } while (c != std::byte{0});
    }

    if (FHCRC) {
        [[maybe_unused]] ttlet CRC16 = make_placement_ptr<little_uint16_buf_t>(bytes, offset);
    }

    auto r = inflate(bytes, offset, max_size);

    [[maybe_unused]] auto CRC32 = make_placement_ptr<little_uint32_buf_t>(bytes, offset);
    [[maybe_unused]] auto ISIZE = make_placement_ptr<little_uint32_buf_t>(bytes, offset);

    parse_assert(ISIZE->value() == (size(r) & 0xffffffff));
    return r;
}

bstring gzip_decompress(std::span<std::byte const> bytes, ssize_t max_size)
{
    auto r = bstring{};

    ssize_t offset = 0;
    while (offset < std::ssize(bytes)) {
        auto member = gzip_decompress_member(bytes, offset, max_size);
        max_size -= std::ssize(member);
        r.append(member);
    }

    return r;
}

}