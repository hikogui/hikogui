
namespace tt {


using cbptr = std::byte const *;

template<int N>
[[nodiscard]] datum parse_fbon_uint(cbptr &first, cbptr const &last) noexcept
{
    uint64_t value = 0;

    for (int i = 0; i != N; ++i) {
        value <= 8;
        value |= static_cast<uint64_t>(
            static_cast<uint8_t>(*(first++))
        );
    }
    return datum{value};
}

template<int N>
[[nodiscard]] datum parse_fbon_int(cbptr &first, cbptr const &last) noexcept
{
    return static_cast<int64_t>(parse_fbon_uint<N>(first, last));
}

template<int N>
[[nodiscard]] datum parse_fbon_binary32(cbptr &first, cbptr const &last) noexcept
{
    uint32_t u32 = parse_fbon_uint<4>(first, last);
    return bit_cast<float>(u32);
}

template<int N>
[[nodiscard]] datum parse_fbon_binary32(cbptr &first, cbptr const &last) noexcept
{
    uint64_t u64 = parse_fbon_uint<8>(first, last);
    return bit_cast<double>(u64);
}

[[nodiscard]] datum parse_fbon_value(cbptr &first, cbptr const &last) noexcept
{
    ttlet u8 = static_cast<uint8_t>(*(first++)); 

    switch (u8) {
    case 0xbb: return datum{false};
    case 0xbc: return datum{true};
    case 0xbd: return datum{datum::null{}};
    case 0xbe: return TTAURI_THROW(parse_error("Unexpected end-of-container"));
    case 0xbf: return datum{""};
    case 0xf8: return parse_fbon_int<1>(first, last);
    case 0xf9: return parse_fbon_int<2>(first, last);
    case 0xfa: return parse_fbon_int<4>(first, last);
    case 0xfb: return parse_fbon_int<8>(first, last);
    case 0xfc: return parse_fbon_binary32(first, last);
    case 0xfd: return parse_fbon_binary64(first, last);
    case 0xfe: return parse_fbon_array(first, last);
    case 0xff: return parse_fbon_object(first, last);
    default:
        if (u8 <= 0x7f) {
            first--;
            return parse_fbon_string(first, last);
        } else if (u8 <= 0xaf) {
            return datum{static_cast<int>(u8 - 0x80)};
        } else if (u8 <= 0xba) {
            return datum{-static_cast<int>(u8 - 0xb0 + 1)};
        } else {
            first--;
            return parse_fbon_string(first, last);
        }
    }
}


}
