

#pragma once

namespace tt {

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


}

