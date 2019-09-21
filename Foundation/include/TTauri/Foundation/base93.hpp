
#include "TTauri/Required/required.hpp"

namespace TTauri {

uint8_t base93_crc(uint128_t data)
{
    uint128_t data = number | (static_cast<uint128_t>(location) << 85);
    uint128_t divider = 0b100101 << 111; // CRC-5-USB polynomial.

    for (int i = 0; i < 111; i++) {
        data %= divider;
        divider >>= 1;
    }
    return static_cast<uint8_t>(data & 0x1f);
}

bool base93_crc_check(uint128_t number, uint32_t location)
{
    auto v = base93_crc((static_cast<uint128_t>(location) << 85) | number);
    return v == 0;
}

std::vector<std::string> base93_decode(std::string_view str)
{
    enum class state_t { Idle, Command, Decoding};
    auto state = state_t::Idle;

    size_t nr_bytes = 0;
    size_t nr_digits = 0;
    size_t nr_numbers = 0;
    size_t nr_characters_per_group = 0;

    std::vector<std::string> messages;
    uint128_t number;
    std::string bytes;

    uint64_t magic_check = 0;

    for (let c: str) {
        if (c == '~') {
            magic_check = 0;
            state = state_t::Command;
        } else {
            switch (state) {
            case state_t::Idle:
                // Ignore all characters.
                break;
            case state_t::Command:
                switch (c) {
                case 'b':
                case '9':
                case '3':
                    magic_check <<= 8;
                    magic_check |= static_cast<uint8_t>(c);
                    break;
                case '{':
                    if (magic_check != 0x623933) { // 'b93'
                        TTAURI_THROW(parse_error("Magic 'base93' check failed."));
                    }
                    state = state_t::Decoding;
                    break;
                case '}':
                    if (!base93_crc_check(number, nr_numbers++)) {
                        TTAURI_THROW(parse_error("CRC error."));
                    }
                    base93_decode_number(bytes, number, nr_digits);
                    messages.push_back(std::move(bytes));
                    bytes.clear();
                    number = 0;
                    nr_digits = 0;
                    state = state_t::Idle;
                    break;
                default:
                    // Unexpected command character, ignore.
                    state = state_t::Idle;
                }
                break;
            case state_t::Decoding:
                if (nr_digits == 13) {
                    if (!base93_crc_check(number, nr_numbers++)) {
                        TTAURI_THROW(parse_error("CRC error."));
                    }
                    base93_decode_number(bytes, number, nr_digits);
                    number = 0;
                    nr_digits = 0;
                }

                if (c >= '!' && c <= '}') {
                    number *= 93;
                    number += (c - '!');
                    nr_digits++;

                } else if (c < '!') {
                    // All non-printable and non-ASCII characters are ignored.
                } else {
                    TTAURI_THROW(parse_error("Non ASCII character found."));
                }
                break;
            }
        }
    }

    return messages;
}

}
