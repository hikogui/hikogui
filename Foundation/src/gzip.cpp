// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/gzip.hpp"
#include "TTauri/Foundation/inflate.hpp"
#include "TTauri/Foundation/endian.hpp"
#include "TTauri/Foundation/placement.hpp"

namespace TTauri {

struct GZIPMemberHeader {
    uint8_t ID1;
    uint8_t ID2;
    uint8_t CM;
    uint8_t FLG;
    little_uint32_buf_t MTIME;
    uint8_t XFL;
    uint8_t OS;
};

static bstring gzip_decompress_member(nonstd::span<std::byte const> bytes, ssize_t &offset, ssize_t max_size)
{
    let header = make_placement_ptr<GZIPMemberHeader>(bytes, offset);

    parse_assert(header->ID1 == 31);
    parse_assert(header->ID2 == 139);
    parse_assert(header->CM == 8);
    parse_assert((header->FLG & 0xe0) == 0); // reserved bits must be zero.
    parse_assert(header->XFL == 2 || header->XFL == 4);
    [[maybe_unused]] let FTEXT = static_cast<bool>(header->FLG & 1);
    let FHCRC = static_cast<bool>(header->FLG & 2);
    let FEXTRA = static_cast<bool>(header->FLG & 4);
    let FNAME = static_cast<bool>(header->FLG & 8);
    let FCOMMENT = static_cast<bool>(header->FLG & 16);

    if (FEXTRA) {
        let XLEN = make_placement_ptr<little_uint16_buf_t>(bytes, offset);
        offset += XLEN->value();
    }

    if (FNAME) {
        std::byte c;
        do {
            parse_assert(offset < ssize(bytes));
            c = bytes[offset++];
        } while (c != std::byte{0});
    }

    if (FCOMMENT) {
        std::byte c;
        do {
            parse_assert(offset < ssize(bytes));
            c = bytes[offset++];
        } while (c != std::byte{0});
    }

    if (FHCRC) {
        [[maybe_unused]] let CRC16 = make_placement_ptr<little_uint16_buf_t>(bytes, offset);
    }

    auto r = inflate(bytes, offset, max_size);

    [[maybe_unused]] auto CRC32 = make_placement_ptr<little_uint32_buf_t>(bytes, offset);
    [[maybe_unused]] auto ISIZE = make_placement_ptr<little_uint32_buf_t>(bytes, offset);

    parse_assert(ISIZE->value() == (size(r) & 0xffffffff));
    return r;
}

bstring gzip_decompress(nonstd::span<std::byte const> bytes, ssize_t max_size)
{
    auto r = bstring{};

    ssize_t offset = 0;
    while (offset < ssize(bytes)) {
        auto member = gzip_decompress_member(bytes, offset, max_size);
        max_size -= ssize(member);
        r.append(member);
    }

    return r;
}

bstring gzip_decompress(FileView const &view, ssize_t max_size)
{
    return gzip_decompress(view.bytes(), max_size);
}

bstring gzip_decompress(URL const &url, ssize_t max_size)
{
    return gzip_decompress(FileView(url), max_size);
}

}