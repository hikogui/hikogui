// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../container/container.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <span>
#include <cstdint>
#include <array>
#include <string>
#include <string_view>
#include <bit>
#include <iterator>
#include <format>

hi_export_module(hikogui.codec.base_n);

hi_export namespace hi::inline v1 {
namespace detail {

struct base_n_alphabet {
    long long radix;
    bool case_insensitive;
    char padding_char;
    std::array<int8_t, 256> int_from_char_table = {};
    std::array<char, 127> char_from_int_table = {};

    /** Construct an alphabet.
     * @param str A null terminated string as a char array.
     * @param case_insensitive The alphabet is case insensitive for decoding.
     * @param padding_char The character used to complete the last block during encoding.
     */
    template<std::size_t StringLength>
    constexpr base_n_alphabet(
        char const (&str)[StringLength],
        bool case_insensitive = StringLength <= 33,
        char padding_char = '\0') noexcept :
        radix(narrow_cast<long long>(StringLength - 1)), case_insensitive(case_insensitive), padding_char(padding_char)
    {
        static_assert(StringLength < 128);

        // Mark the int_from_char_table to have invalid characters.
        for (long long i = 0; i != 256; ++i) {
            int_from_char_table[i] = -2;
        }

        // Mark white-space in the int_from_char_table as white-space.
        int_from_char_table[std::bit_cast<uint8_t>(' ')] = -1;
        int_from_char_table[std::bit_cast<uint8_t>('\t')] = -1;
        int_from_char_table[std::bit_cast<uint8_t>('\r')] = -1;
        int_from_char_table[std::bit_cast<uint8_t>('\n')] = -1;
        int_from_char_table[std::bit_cast<uint8_t>('\f')] = -1;

        if (padding_char != 0) {
            int_from_char_table[std::bit_cast<uint8_t>(padding_char)] = -1;
        }

        for (long long i = 0; i != radix; ++i) {
            auto c = str[i];
            char_from_int_table[i] = c;

            int_from_char_table[std::bit_cast<uint8_t>(c)] = narrow_cast<int8_t>(i);
            if constexpr (StringLength <= 33) {
                // Add an extra entry for case folded form.
                if (c >= 'a' && c <= 'z') {
                    int_from_char_table[narrow_cast<uint8_t>((c - 'a') + 'A')] = narrow_cast<int8_t>(i);
                } else if (c >= 'A' && c <= 'Z') {
                    int_from_char_table[narrow_cast<uint8_t>((c - 'A') + 'a')] = narrow_cast<int8_t>(i);
                }
            }
        }
    }

    /** Get a character from an integer.
     * The integer must be in range of 0 to modula (exclusive).
     */
    constexpr char char_from_int(int8_t x) const noexcept
    {
        hi_axiom(x < radix);
        return char_from_int_table[x];
    }

    constexpr int8_t int_from_char(char c) const noexcept
    {
        return int_from_char_table[std::bit_cast<uint8_t>(c)];
    }
};

constexpr auto base2_alphabet = base_n_alphabet{"01"};

constexpr auto base8_alphabet = base_n_alphabet{"01234567"};

constexpr auto base10_alphabet = base_n_alphabet{"0123456789"};

constexpr auto base16_alphabet = base_n_alphabet{"0123456789ABCDEF"};

constexpr auto base32_rfc4648_alphabet = base_n_alphabet{"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"};

constexpr auto base32hex_rfc4648_alphabet = base_n_alphabet{"0123456789ABCDEFGHIJKLMNOPQRSTUV"};

constexpr auto base64_rfc4648_alphabet =
    base_n_alphabet{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/", false, '='};

constexpr auto base64url_rfc4648_alphabet =
    base_n_alphabet{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_", false, '='};

constexpr auto base85_rfc1924_alphabet =
    base_n_alphabet{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<=>?@^_`{|}~"};

constexpr auto base85_btoa_alphabet =
    base_n_alphabet{"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu"};

} // namespace detail

template<detail::base_n_alphabet Alphabet, int CharsPerBlock, int BytesPerBlock>
class base_n {
public:
    constexpr static detail::base_n_alphabet alphabet = Alphabet;
    constexpr static char padding_char = alphabet.padding_char;
    constexpr static long long radix = alphabet.radix;
    constexpr static long long bytes_per_block = BytesPerBlock;
    constexpr static long long chars_per_block = CharsPerBlock;
    static_assert(bytes_per_block != 0, "radix must be 16, 32, 64 or 85");
    static_assert(chars_per_block != 0, "radix must be 16, 32, 64 or 85");

    template<typename T>
    constexpr static T int_from_char(char c) noexcept
    {
        return narrow_cast<T>(alphabet.int_from_char(c));
    }

    template<typename T>
    constexpr static char char_from_int(T x) noexcept
    {
        return alphabet.char_from_int(narrow_cast<int8_t>(x));
    }

    /** Encode bytes into a string.
     *
     * @param ptr Pointer
     * @param last Beyond the last byte.
     * @param output
     */
    template<typename ItIn, typename ItOut>
    constexpr static void encode(ItIn ptr, ItIn last, ItOut output)
    {
        long long byte_index_in_block = 0;
        long long block = 0;

        while (ptr != last) {
            // Construct a block in big endian.
            hilet shift = 8 * ((bytes_per_block - 1) - byte_index_in_block);
            block |= static_cast<long long>(*(ptr++)) << shift;

            if (++byte_index_in_block == bytes_per_block) {
                encode_block(block, bytes_per_block, output);
                block = 0;
                byte_index_in_block = 0;
            }
        }

        if (byte_index_in_block != 0) {
            encode_block(block, byte_index_in_block, output);
        }
    }

    /** Encode bytes into a string.
     *
     * @param first An iterator pointing to the first byte to encode.
     * @param last An iterator pointing to one beyond the last byte to encode.
     * @return The data encoded as a string.
     */
    template<typename ItIn>
    static std::string encode(ItIn first, ItIn last) noexcept
    {
        std::string r;
        encode(first, last, std::back_inserter(r));
        return r;
    }

    /** Encode bytes into a string.
     *
     * @param bytes A span of bytes to encode.
     * @return The data encoded as a string.
     */
    constexpr static std::string encode(std::span<std::byte const> bytes) noexcept
    {
        return encode(begin(bytes), end(bytes));
    }

    /** Decodes a UTF-8 string into bytes.
     *
     * @param ptr An iterator inside a UTF-8 string to the start of a base-n encoded data.
     * @param last An iterator beyond the encoded data.
     * @param output An output iterator to a std::byte buffer.
     * @return An iterator pointing on the first invalid character or last.
     */
    template<typename ItIn, typename ItOut>
    constexpr static ItIn decode(ItIn ptr, ItIn last, ItOut output)
    {
        int char_index_in_block = 0;
        long long block = 0;

        for (; ptr != last; ++ptr) {
            hilet digit = int_from_char<long long>(*ptr);
            if (digit == -1) {
                // Whitespace is ignored.
                continue;

            } else if (digit == -2) {
                // Other character means end
                return ptr;

            } else {
                block *= radix;
                block += digit;

                if (++char_index_in_block == chars_per_block) {
                    decode_block(block, chars_per_block, output);
                    block = 0;
                    char_index_in_block = 0;
                }
            }
        }

        if (char_index_in_block != 0) {
            // pad the block with zeros.
            for (auto i = char_index_in_block; i != chars_per_block; ++i) {
                block *= radix;
            }
            decode_block(block, char_index_in_block, output);
        }
        return ptr;
    }

    static bstring decode(std::string_view str)
    {
        auto r = bstring{};
        auto i = decode(begin(str), end(str), std::back_inserter(r));
        hi_check(i == end(str), "base-n encoded string not completely decoded");
        return r;
    }

private:
    template<typename ItOut>
    static void encode_block(long long block, long long nr_bytes, ItOut output) noexcept
    {
        hilet padding = bytes_per_block - nr_bytes;

        // Construct a block in little-endian, using easy division/modulo.
        auto char_block = std::string{};
        for (long long i = 0; i != chars_per_block; ++i) {
            hilet v = block % radix;
            block /= radix;

            if (i < padding) {
                hi_assume(v != 0);
                if (padding_char != 0) {
                    char_block += padding_char;
                }
            } else {
                char_block += char_from_int(v);
            }
        }

        // A block should be output as a big-endian radix-number.
        std::copy(rbegin(char_block), rend(char_block), output);
    }

    template<typename ItOut>
    constexpr static void decode_block(long long block, long long nr_chars, ItOut output)
    {
        hilet padding = chars_per_block - nr_chars;

        if (block and bytes_per_block == padding) {
            throw parse_error("Invalid number of character to decode.");
        }

        // Construct a block in little-endian, using easy division/modulo.
        for (long long i = 0; i != (bytes_per_block - padding); ++i) {
            hilet shift = 8 * ((bytes_per_block - 1) - i);
            hilet byte = static_cast<std::byte>((block >> shift) & 0xff);

            *(output++) = byte;
        }

        // The output data will not contain the padding.
    }
};

// Alphabet, CharsPerBlock, BytesPerBlock
hi_export using base2 = base_n<detail::base2_alphabet, 8, 1>;
hi_export using base8 = base_n<detail::base8_alphabet, 8, 3>;
hi_export using base16 = base_n<detail::base16_alphabet, 2, 1>;
hi_export using base32 = base_n<detail::base32_rfc4648_alphabet, 8, 5>;
hi_export using base32hex = base_n<detail::base32hex_rfc4648_alphabet, 8, 5>;
hi_export using base64 = base_n<detail::base64_rfc4648_alphabet, 4, 3>;
hi_export using base64url = base_n<detail::base64url_rfc4648_alphabet, 4, 3>;
hi_export using base85 = base_n<detail::base85_rfc1924_alphabet, 5, 4>;
hi_export using ascii85 = base_n<detail::base85_btoa_alphabet, 5, 4>;

} // namespace hi::inline v1
