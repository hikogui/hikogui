// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/base93.hpp"
#include "TTauri/Diagnostic/exceptions.hpp"
#include "TTauri/Foundation/bigint.hpp"
#include <algorithm>

using namespace std::literals;

namespace TTauri {

static uint8_t base93_crc(uint128_t number) noexcept
{
    auto divider = static_cast<uint128_t>(0b100101) << 111; // CRC-5-USB polynomial.
    auto top_bit = (static_cast<uint128_t>(0b100000) << 111) - 1; // CRC-5-USB polynomial.

    // Number already includes the 5 CRC/padding bits.
    // Continue until the dividend is zero.
    while (number >= 32) {
        if (top_bit < number) {
            // Align the top bit of the divider with the number before dividing.
            number ^= divider;
        }

        divider >>= 1;
        top_bit >>= 1;
    }
    return static_cast<uint8_t>(number & 0x1f);
}

static bool base93_crc_check(uint128_t number, size_t location) noexcept
{
    auto v = base93_crc((static_cast<uint128_t>(location) << 85) | number);
    return v == 0;
}

static uint128_t base93_add_crc(uint128_t number, size_t location) noexcept
{
    auto v = base93_crc((static_cast<uint128_t>(location) << 85) | number);
    return number | v;
}

/*!
 * | digits | bits   | bytes  | unused |
 * | ------:| ------:| ------:| ------:|
 * |      2 |     13 |      1 |      0 |
 * |      4 |     26 |      2 |      5 |
 * |      5 |     32 |      3 |      3 |
 * |      6 |     39 |      4 |      2 |
 * |      7 |     45 |      5 |      0 |
 * |      9 |     58 |      6 |      5 |
 * |     10 |     65 |      7 |      4 |
 * |     11 |     71 |      8 |      2 |
 * |     12 |     78 |      9 |      1 |
 * |     13 |     85 |     10 |      0 |
 */
static size_t base93_nr_digits_to_nr_bytes(size_t nr_digits)
{
    switch (nr_digits) {
    case 2: return 1;
    case 4: return 2;
    case 5: return 3;
    case 6: return 4;
    case 7: return 5;
    case 9: return 6;
    case 10: return 7;
    case 11: return 8;
    case 12: return 9;
    case 13: return 10;
    default:
        TTAURI_THROW(parse_error("Unexpected number of characters at end of message"));
    }
}

static size_t base93_nr_bytes_to_nr_digits(size_t nr_bytes) noexcept
{
    switch (nr_bytes) {
    case 1: return 2;
    case 2: return 4;
    case 3: return 5;
    case 4: return 6;
    case 5: return 7;
    case 6: return 9;
    case 7: return 10;
    case 8: return 11;
    case 9: return 12;
    case 10: return 13;
    default:
        no_default;
    }
}

static void base93_decode_number(bstring &message, uint128_t number, size_t nr_digits)
{
    // Strip off CRC
    number >>= 5;

    let nr_bytes = base93_nr_digits_to_nr_bytes(nr_digits);
    for (size_t i = 0; i < nr_bytes; i++) {
        message += static_cast<std::byte>(static_cast<uint8_t>(number & 0xff));
        number >>= 8;
    }
}

static uint128_t base93_bytes_to_number(bstring_view bytes) noexcept
{
    auto number = uint128_t{0};

    for (auto i = static_cast<int>(bytes.size()) - 1; i >= 0; i--) {
        number <<= 8;
        number |= static_cast<uint8_t>(bytes.at(i));
    }

    // Add empty CRC.
    number <<= 5;
    return number;
}

static std::string base93_encode_number(uint128_t number, size_t nr_bytes) noexcept
{
    let nr_digits = base93_nr_bytes_to_nr_digits(nr_bytes);
    auto r = std::string(nr_digits, '!');

    for (auto i = static_cast<int>(nr_digits) - 1; i >= 0; i--) {
        let [quotient, remainder] = div(number, 93);
        r.at(i) = static_cast<char>(remainder) + '!';
        number = quotient;
    }

    return r;
}

static std::string base93_encode_bytes(bstring_view bytes, size_t location) noexcept
{
    let number = base93_bytes_to_number(bytes);
    let number_with_crc = base93_add_crc(number, location);
    return base93_encode_number(number_with_crc, bytes.size());
}

std::string base93_encode(bstring_view message) noexcept
{
    auto str = "~b93{"s;

    size_t nr_numbers = 0;
    size_t offset = 0;
    auto number = uint128_t{0};

    while (offset < message.size()) {
        let nr_bytes = std::min(static_cast<size_t>(10), message.size() - offset);
        str += base93_encode_bytes(message.substr(offset, nr_bytes), nr_numbers++);
        offset += nr_bytes;
    }

    str += "~}";
    return str;
}


bstring base93_decode(std::string_view str, size_t &offset)
{
    enum class state_t { Idle, Command, Decoding, Finished };
    auto state = state_t::Idle;

    size_t nr_digits = 0;
    size_t nr_numbers = 0;

    bstring message;
    auto number = uint128_t{0};

    uint64_t magic_check = 0;

    while (offset < str.size()) {
        let c = str.at(offset++);

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
                        TTAURI_THROW(parse_error("Magic 'base93' check failed.")
                            .set<"location"_tag>(offset - 1)
                        );
                    }
                    state = state_t::Decoding;
                    break;

                case '}':
                    if (nr_digits > 0) {
                        // Decode the last bit of the message.
                        if (!base93_crc_check(number, nr_numbers++)) {
                            TTAURI_THROW(parse_error("CRC error.")
                                .set<"location"_tag>(offset - 1)
                            );
                        }
                        try {
                            base93_decode_number(message, number, nr_digits);
                        } catch (error &e) {
                            e.set<"location"_tag>(offset - 1);
                            throw;
                        }
                    }
                    return message;

                default:
                    // Unexpected command character, ignore.
                    state = state_t::Idle;
                }
                break;

            case state_t::Decoding:
                if (nr_digits == 13) {
                    if (!base93_crc_check(number, nr_numbers++)) {
                        TTAURI_THROW(parse_error("CRC error.")
                            .set<"location"_tag>(offset - 1)
                        );
                    }
                    try {
                        base93_decode_number(message, number, nr_digits);
                    } catch (error &e) {
                        e.set<"location"_tag>(offset - 1);
                        throw;
                    }
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
                    TTAURI_THROW(parse_error("Non ASCII character found.")
                        .set<"location"_tag>(offset - 1)
                    );
                }
                break;
            }
        }
    }

    TTAURI_THROW(parse_error("Incomplete base-93 message.")
        .set<"location"_tag>(offset - 1)
    );
}

}