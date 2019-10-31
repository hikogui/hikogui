// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/base93.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/bigint.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include <algorithm>

using namespace std::literals;

namespace TTauri {


static uint8_t base93_crc(ubig128 number) noexcept
{
    return number.crc(0b100101);
}

static bool base93_crc_check(ubig128 number) noexcept
{
    auto v = base93_crc(number >> 5);
    return v == (number & 0x1f);
}

static ubig128 base93_add_crc(ubig128 number) noexcept
{
    auto v = base93_crc(number >> 5);
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
    case 0: return 0;
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

static void base93_number_to_bytes(bstring &message, ubig128 number, size_t nr_digits)
{
    // Strip off CRC
    number >>= 5;

    let nr_bytes = base93_nr_digits_to_nr_bytes(nr_digits);
    for (size_t i = 0; i < nr_bytes; i++) {
        message += static_cast<std::byte>(static_cast<uint8_t>(number & 0xff));
        number >>= 8;
    }
}

static ubig128 base93_bytes_to_number(bstring_view bytes) noexcept
{
    auto number = ubig128{0};

    for (ssize_t i = to_signed(bytes.size()) - 1; i >= 0; i--) {
        number <<= 8;
        number |= static_cast<uint8_t>(bytes.at(i));
    }

    // Add empty CRC.
    number <<= 5;
    return number;
}

static std::string base93_number_to_characters(ubig128 number, size_t nr_bytes) noexcept
{
    static auto oneOver93 = bigint_reciprocal<uint64_t,4>(93);

    let nr_digits = base93_nr_bytes_to_nr_digits(nr_bytes);
    auto r = std::string(nr_digits, '!');

    for (ssize_t i = to_signed(nr_digits) - 1; i >= 0; i--) {
        let [quotient, remainder] = div(number, 93, oneOver93);
        r.at(i) = static_cast<char>(remainder) + '!';
        number = quotient;
    }

    return r;
}

static std::string base93_encode_bytes(bstring_view bytes) noexcept
{
    let number = base93_bytes_to_number(bytes);
    let number_with_crc = base93_add_crc(number);
    return base93_number_to_characters(number_with_crc, bytes.size());
}

std::string base93_encode(bstring_view message) noexcept
{
    // Split the message with a line feed every 76 characters.
    // Make sure to split numbers, so that CRCs will detect missing lines.
    constexpr int maximumLineLength = 76;

    let totalNumbers = (message.size() + 9) / 10;
    let totalDigits = totalNumbers * 13;
    let totalLineSeperators = (totalDigits + 5 + maximumLineLength) / maximumLineLength;
    let totalCharacters = totalDigits + totalLineSeperators + 5;

    std::string str;
    str.reserve(totalCharacters);

    str += "~b93";
    size_t offset = 0;
    size_t currentLineLength = 0;
    while (offset < message.size()) {
        let nr_bytes = std::min(static_cast<size_t>(10), message.size() - offset);

        let digits = base93_encode_bytes(message.substr(offset, nr_bytes));

        let splitPosition = std::min(digits.size(), maximumLineLength - currentLineLength);
        str += digits.substr(0, splitPosition);
        if (splitPosition < digits.size()) {
            str += '\n';
            str += digits.substr(splitPosition);
            currentLineLength = digits.size() - splitPosition;
        } else {
            currentLineLength += splitPosition;
        }

        offset += nr_bytes;
    }

    // Always add the closing ~ on the current line.
    str += '~';
    return str;
}


std::optional<bstring> base93_decode(std::string_view str, size_t &offset)
{
    enum class state_t { Idle, FoundTilde, FoundB, Found9, Decoding };
    auto state = state_t::Idle;

    size_t nr_digits = 0;
    bstring message;
    auto number = ubig128{0};

    while (offset < str.size()) {
        let c = str.at(offset++);

        switch (state) {
        case state_t::Idle:
            state = c == '~' ? state_t::FoundTilde : state_t::Idle;
            break;

        case state_t::FoundTilde:
            state = c == 'b' ? state_t::FoundB : state_t::Idle;
            break;

        case state_t::FoundB:
            state = c == '9' ? state_t::Found9 : state_t::Idle;
            break;

        case state_t::Found9:
            state = c == '3' ? state_t::Decoding : state_t::Idle;
            break;

        case state_t::Decoding:
            if (nr_digits == 13 || c == '~') {
                if (!base93_crc_check(number)) {
                    let [line, column] = count_line_and_columns(str.begin(), str.begin() + offset-1);
                    TTAURI_THROW(parse_error("CRC error.")
                        .set<"line"_tag>(line).set<"column"_tag>(column)
                    );
                }
                try {
                    base93_number_to_bytes(message, number, nr_digits);
                } catch (error &e) {
                    let [line, column] = count_line_and_columns(str.begin(), str.begin() + offset-1);
                    e.set<"line"_tag>(line).set<"column"_tag>(column);
                    throw;
                }
                number = 0;
                nr_digits = 0;
            }

            if (c >= '!' && c <= '}') {
                number *= 93;
                number += (c - '!');
                nr_digits++;

            } else if (c == '~') {
                return {std::move(message)};

            } else if (static_cast<uint8_t>(c) >= 128) {
                let [line, column] = count_line_and_columns(str.begin(), str.begin() + offset-1);
                TTAURI_THROW(parse_error("Non ASCII character found.")
                    .set<"line"_tag>(line).set<"column"_tag>(column)
                );
            }
            break;
        }
    }

    if (state == state_t::Decoding) {
        let [line, column] = count_line_and_columns(str.begin(), str.begin() + offset-1);
        TTAURI_THROW(parse_error("Incomplete base-93 message.")
            .set<"line"_tag>(line).set<"column"_tag>(column)
        );
    }
    return {};
}

}