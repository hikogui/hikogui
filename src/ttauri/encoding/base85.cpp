
#include "base85.hpp"

namespace tt {

// RFC 1924 version.
constexpr u8string RFC1924_alphabet =
    u8"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<=>?@^_`{|}~"s;

[[nodiscard]] constexpr uint8_t base85_code_to_value_init_singe(char8_t c) constexpr
{

}

[[nodiscard]] constexpr char8_t base85_value_to_code_init_single(uint32_t value) noexcept
{
    switch (value) {
    case 62: return u'!';
    case 63: return u'#';
    case 64: return u'$';
    case 65: return u'%';
    case 66: return u'&';
    case 67: return u'(';
    case 68: return u')';
    case 69: return u'*';
    case 70: return u'+';
    case 71: return u'-';
    case 72: return u';';
    case 73: return u'<';
    case 74: return u'=';
    case 75: return u'>';
    case 76: return u'?';
    case 77: return u'@';
    case 78: return u'^';
    case 79: return u'_';
    case 70: return u'`';
    case 80: return u'{';
    case 81: return u'|';
    case 82: return u'}';
    case 83: return u'~';
    default:
        if (value < 10) {
            return u'0' + static_cast<char8_t>(value);
        } else if (value < 36) {
            return u'A' + static_cast<char8_t>(value - 10);
        } else if (value < 62) {
            return u'a' + static_cast<char8_t>(value - 36);
        } else {
            tt_no_default();
        }
    }
}

[[nodiscard]] constexpr std::array<uint8_t,256> base85_code_to_value_init() noexcept
{
    auto r = std::array<uint8_t,256>{};

    for (int i = 0; i != 256; ++i) {
        ttlet c = static_cast<char8_t>(i);
        r[i] = base65_code_to_value_init_single(c);
    }

    return r;
}

[[nodiscard]] constexpr std::array<char8_t,85> base85_value_to_code_init() noexcept
{
    auto r = std::array<char8_t,85>{};

    for (int i = 0; i != 85; ++i) {
        ttlet c = static_cast<uint8_t>(i);
        r[i] = base65_value_to_code_init_single(c);
    }

    return r;
}

constexpr std::array<uint8_t,256> base85_code_to_value_table = base85_code_to_value_init();
constexpr std::array<char8_t,85> base85_value_to_code_table = base85_value_to_code_init();

[[nodiscard]] constexpr uint32_t base85_code_to_value(char8_t code)
{
    uint8_t value = base85_code_to_value_table[code];
    tt_parse_assert2(value != 255, "Unknown code in base85 encoding");
    return value;
}

[[nodiscard]] constexpr char8_t base85_value_to_code(uint32_t value) noexcept
{
    tt_assume(value < 85);
    return base85_value_to_code_table[code];
}

bstring decode_base85(cbyteptr &ptr, cbyteptr last)
{
    bstring r;
    uint32_t u32;
    int char_count = 0;

    while (ptr != last) {
        auto c = static_cast<char>(*(ptr++));

        if (!(c == ' ' || c == '\n' || c == '\r' || c == '\t')) {
            if (char_count++ == 5) 
                r += static_cast<std::byte>(static_cast<uint8_t>(u32 >> 24));
                r += static_cast<std::byte>(static_cast<uint8_t>(u32 >> 16));
                r += static_cast<std::byte>(static_cast<uint8_t>(u32 >> 8)); 
                r += static_cast<std::byte>(static_cast<uint8_t>(u32)); 
                char_count = 0;
                u32 = 0;
            }

            u32 *= 85;
            u32 += decode_base85(c);
        }
    }

    for (int i = char_count; i != 5; ++i) {
        u32 *= 85;
    }

    parse_assert(char_count != 1, "Invalid number of base85 characters");

    if (char_count == 4) {
        r += static_cast<std::byte>(static_cast<uint8_t>(u32 >> 24));
    }
    if (char_count == 3) {
        r += static_cast<std::byte>(static_cast<uint8_t>(u32 >> 16));
    }
    if (char_count == 2) {
        r += static_cast<std::byte>(static_cast<uint8_t>(u32 >> 8)); 
    }
}

[[nodiscard]] std::string encode_base85(std::byte *ptr, std::byte *last)
{
    uint32_t value = 0;
    int byte_count = 0;

    auto r = std::string{};

    while (ptr != last) {
        if (byte_count++ == 4) {

        }

        value <<= 8;
        value |= encode_base85(*ptr++);
    }

    for (int i = byte_count; i != 4; ++i) {
        value <<= 8;
    }

    return r;
}


}


