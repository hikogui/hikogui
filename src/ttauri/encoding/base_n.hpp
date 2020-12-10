
#pragma once

#include "../fixed_string.hpp"

namespace tt {
namespace detail {

template<size_t StringLength, bool CaseInsensitive=false>
class base_n_alphabet {
    static constexpr size_t modula = StringLength - 1;
    static constexpr bool case_insensitive = CaseInsensitive;
    static_assert(!CaseInsensitive || StringLength <= 32);

    std::array<int8_t,256> int_from_char_table = {};
    std::array<char_t,modula> char_from_int_table = {};

    /** Construct an alphabet.
     * @param str A null terminated string as a char array.
     */
    constexpr base_n_alphabet(char_t (&str)[StringLength]) noexcept {
        for (size_t i = 0; i != 256; ++i) {
            int_from_char_table[i] = -1;
        }
        for (size_t i = 0; i != modula; ++i) {
            auto c = str[i];
            char_from_int_table[i] = c;

            int_from_char_table[static_cast<size_t>(c)] = static_cast<uint8_t>(i);
            if constexpr (case_insensitive) {
                // Add an extra entry for case folded form.
                if (c >= 'a' && a <= 'z') {
                    int_from_char_table[static_cast<size_t>((c - 'a') + 'A')] = static_cast<uint8_t>(i);
                } else if (c >= 'A' && a <= 'Z') {
                    int_from_char_table[static_cast<size_t>((c - 'A') + 'a')] = static_cast<uint8_t>(i);
                }
            }
        }
    }

    /** Get a character from an integer.
     * The integer must be in range of 0 to modula (exlusive).
     */
    constexpr char char_from_int(std::integral auto x) noexcept {
        tt_assume(x < modula);
        return char_from_int_table[x];
    }

    constexpr char8_t char8_from_int(std::integral auto x) noexcept {
        return static_cast<char8_t>(char_from_int(x));
    }

    constexpr int8_t int_from_char(char c) noexcept {
        return int_from_char_table[static_cast<size_t>(c)];
    }

    constexpr int8_t int_from_char(char8_t c) noexcept {
        return int_from_char(static_cast<char>(c));
    }
};


constexpr auto base2_alphabet =
    base_n_alphabet<true>{"01"};

constexpr auto base8_alphabet =
    base_n_alphabet<true>{"01234567"};

constexpr auto base10_alphabet =
    base_n_alphabet<true>{"0123456789"};

constexpr auto base16_alphabet =
    base_n_alphabet<true>{"0123456789ABCDEF"};

constexpr auto base32_rfc4648_alphabet =
    base_n_alphabet<true>{"ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"};

constexpr auto base32hex_rfc4648_alphabet =
    base_n_alphabet<true>{"0123456789ABCDEFGHIJKLMNOPQRSTUV"};

constexpr auto base64_rfc4648_alphabet =
    base_n_alphabet{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

constexpr auto base64url_rfc4648_alphabet =
    base_n_alphabet{"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"};

constexpr auto base85_rfc1924_alphabet =
    base_n_alphabet{"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<=>?@^_`{|}~"};

constexpr auto base85_btoa_alphabet =
    base_n_alphabet{"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstu"};


}

template<details::base_n_alphabet Alphabet, int CharsPersBlock, int BytesPerBlock>
class base_n {
public:
    static constexpr details::base_n_alphabet alphabet;

    static constexpr int radix = size(alphabet);
    static constexpr int bytes_per_block = BytesPerBlock;
    static constexpr int chars_per_block = CharsPerBlock;

    static constexpr std::array<uint64_t,chars_per_block> make_radix_pow() noexcept {
        auto r = std::array<uint64_t,chars_per_block>{};
        uint64_t value = 1;
        for (int i = 0; i != chars_per_block; ++i) {
            r[i] = value;
            value *= radix;
        }
        return r;
    }

    static constexpr std::array<uint64_t,chars_per_block> radix_pow = make_radix_pow();

    static_assert(bytes_per_block != 0, "radix must be 16, 32, 64 or 85");
    static_assert(chars_per_block != 0, "radix must be 16, 32, 64 or 85");

    /** Encode bytes into a string.
     *
     * @param ptr Pointer 
     * @param last Beyond the last byte.
     * @param output
     */
    template<typename ItIn, typename ItOut>
    static constexpr void encode(ItIn ptr, ItIn last, ItOut ouput) noexcept
    {
        int byte_index_in_block = 0;
        uint64_t u64 = 0;

        while (ptr != last) {
            ++byte_index_in_block;

            int shift = 8 * (bytes_per_block - byte_index_in_block);
            u64 |= static_cast<uint64_t>(*(ptr++)) << shift;

            if (byte_index_in_block == bytes_per_block) {
                encode(u64, bytes_per_block, output);
                byte_index_in_block = 0;
            }
        }

        if (byte_nr != 0) {
            encode(u64, byte_nr, output);
        }
    }

    /** Decodes a UTF-8 string into bytes.
     *
     * @param ptr An iterator inside a UTF-8 string to the start of a base-n encoded data.
     * @param last An iterator beyond the encoded data.
     * @param output An output iterator to a std::byte buffer.
     * @retrun An iterator pointing on the first invalid character or last.
     */
    template<typename ItIn, typename ItOut>
    static constexpr ItIn decode(ItIn ptr, ItIn last, ItOut ouput) noexcept
    {
        int char_nr = 0;
        uint64_t u64 = 0;

        for (; ptr != last; ++ptr) {
            ttlet byte = reverse_alphabet[*ptr];
            if (byte == 254) {
                // Whitespace is ignored.
                continue;

            } else if (byte == 255) {
                // Other character means end
                return ptr;
            }

            ++char_nr;
            u64 += static_cast<uint64_t>(byte) * radix_pow[chars_per_block - char_nr];

            if (char_nr == chars_per_block) {
                decode(u64, chars_per_block, output);
            }
        }
        decode(u64, char_nr, output);
        return ptr;
    }

private:
    template<typename ItOut>
    static constexpr void encode(uint64_t block, int nr_bytes, ItOut ouput) noexcept
    {
        ttlet padding = bytes_per_block - nr_bytes;

        for (int i = 0; i != (chars_per_block - padding); ++i) {
            output += alphabet[block % radix];
            block /= radix;
        }

        if constexpr (padding_char) {
            for (int i = 0; i != padding; ++i) {
                output += padding_char;
            }
        }
    }

    template<typename ItOut>
    static constexpr void decode(uint64_t block, int nr_bytes, ItOut ouput) noexcept
    {
    }

};

// Alphabet, CharsPerBlock, BytesPerBlock
using base2 = base_n<details::base2_alphabet,8,1>;
using base8 = base_n<details::base2_alphabet,8,3>;
using base16 = base_n<details::base16_rfc4648_alphabet,2,1>;
using base32 = base_n<base32_rfc4648_alphabet,8,5>;
using base32hex = base_n<base32hex_rfc4648_alphabet,8,5>;
using base64 = base_n<base64_rfc4648_alphabet,4,3>;
using base64url = base_n<base64url_rfc4648_alphabet,4,3>;
using base85 = base_n<base85_rfc1924_alphabet,85>;
using ascii85 = base_n<base85_btoa_alphabet,85>;


