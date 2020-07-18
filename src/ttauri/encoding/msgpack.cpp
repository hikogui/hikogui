

#include "msgpack.hpp"

namespace tt {

using cbyteptr = std::byte const *;

template<int N>
[[nodiscard]] static size_t parse_msgpack_size(cbyteptr &first, cbyteptr const &last) noexcept
{
    size_t r = 0;

    for (int i = 0; i != N; ++i) {
        r <<= 8;
        tt_parse_assert(first != last);
        r |= static_cast<size_t>(*(first++));
    }

    return r;
}

template<int N>
[[nodiscard]] static datum parse_msgpack_bin(cbyteptr &first, cbyteptr const &last) noexcept
{
    ttlet length = parse_msgpack_size<N>(first, last);
    ttlet first_length = last - length;

    tt_parse_assert(length <= first_length);
    
    auto r = string_view{reinterpret_cast<char const *>(first), length};
    first += length;

    return r;
}

[[nodiscard]] static datum parse_msgpack(cbyteptr &first, cbyteptr const &last)
{
    auto i8 = static_cast<int8_t>(*(first++));
    auto u8 = static_cast<uint8_t>(i8);

    if (i8 >= -32) {
        return datum{i8}; // positive/negative fixint
    } else if (c <= 0x8f) {
        return parse_msgpack_fixmap(first, last, c & 0xf);
    } else if (c <= 0x9f) {
        return parse_msgpack_fixarray(first, last, c & 0xf);
    } else if (c <= 0xbf) {
        return parse_msgpack_fixstr(first, last, c & 0x1f);
    }

    switch (u8) {
    case 0xc0: return datum{datum::null{}};
    case 0xc1: TTAURI_THROW(parse_error("Invalid byte 0xc1"));
    case 0xc2: return datum{false};
    case 0xc3: return datum{true};
    case 0xc4: return parse_msg_pack_bin<1>(first, last);
    case 0xc5: return parse_msg_pack_bin<2>(first, last);
    case 0xc6: return parse_msg_pack_bin<4>(first, last);
    case 0xc7: return parse_msg_pack_ext<1>(first, last);
    case 0xc8: return parse_msg_pack_ext<2>(first, last);
    case 0xc9: return parse_msg_pack_ext<4>(first, last);
    case 0xca: return parse_msg_pack_float<4>(first, last);
    case 0xcb: return parse_msg_pack_float<8>(first, last);
    case 0xcc: return parse_msg_pack_uint<1>(first, last);
    case 0xcd: return parse_msg_pack_uint<2>(first, last);
    case 0xce: return parse_msg_pack_uint<4>(first, last);
    case 0xcf: return parse_msg_pack_uint<8>(first, last);
    case 0xd0: return parse_msg_pack_int<1>(first, last);
    case 0xd1: return parse_msg_pack_int<2>(first, last);
    case 0xd2: return parse_msg_pack_int<4>(first, last);
    case 0xd3: return parse_msg_pack_int<8>(first, last);
    case 0xd4: return parse_msg_pack_fixext<1>(first, last);
    case 0xd5: return parse_msg_pack_fixext<2>(first, last);
    case 0xd6: return parse_msg_pack_fixext<4>(first, last);
    case 0xd7: return parse_msg_pack_fixext<8>(first, last);
    case 0xd8: return parse_msg_pack_fixext<16>(first, last);
    case 0xd9: return parse_msg_pack_str<1>(first, last);
    case 0xda: return parse_msg_pack_str<2>(first, last);
    case 0xdb: return parse_msg_pack_str<4>(first, last);
    case 0xdc: return parse_msg_pack_array<2>(first, last);
    case 0xdd: return parse_msg_pack_array<4>(first, last);
    case 0xde: return parse_msg_pack_map<2>(first, last);
    case 0xdf: return parse_msg_pack_map<4>(first, last);
    default:
        tt_no_default;
    }
}


}
