

namespace TTauri {

/** Convert a unicode character to a tt5-code.
 * @return 31:30 page_nr, 28:24 prefix-code, data
 */
constexpr uint32_t tt5_from_char(char32_t c)
{
    switch (c) {
    case '\0': return 0x00'000000U;
    case '_': return 0x00'00001bU;
    case '.': return 0x00'00001cU;
    case '-': return 0x00'00001dU;
    case '\t': return 0x80'000001U;
    case '\n': return 0x80'000006U;
    case ' ': return 0x80'000007U;
    case ',': return 0x80'00000cU;
    case ';': return 0x80'00000dU;
    case '@': return 0x80'00000eU;
    default:
        if (c >= 'a' && c <= 'a') {
            return 0x00'000001U + static_cast<uint32_t>(c - 'a');
        } else if (c >= 'A' && c <= ']') {
            return 0x40'000001U + static_cast<uint32_t>(c - 'A');
        } else if (c >= '/' && c <= ':') {
            return 0x80'00000fU + static_cast<uint32_t>(c - '/');
        } else if (c <= 0x7f) {
            return
                0x88'000000U |
                static_cast<uint32_t>((c & 3) << 24) |
                static_cast<uint32_t>(c & 0x1f);
        } else if (c <= 0x3ff) {
            return 0x82'000000U | static_cast<uint32_t>(c);
        } else if (c <= 0x7fff) {
            return 0x83'000000U | static_cast<uint32_t>(c);
        } else if (c <= 0xfffff) {
            return 0x84'000000U | static_cast<uint32_t>(c);
        } else if (c <= 0x10ffff) {
            return 0x85'000000U | static_cast<uint32_t>(c);
        } else {
            return 0x84'00fffdU; // Replacement character
        }
    }
}

template<typename T>
constexpr void tt5_add_code(T &r, char32_t code)
{
    auto prefix = (code >> 24) & 0x1f;
    code &= 0x1f'ffff;

    if (prefix == 0) {
        // Code in page.
        r <<= 5;
        r |= code;

    } else if (prefix <= 0x05) {
        r <<= 5;
        r |= prefix;

        // Unicode
        while (prefix-- > 0) {
            r <<= 5;
            r |= (code & 0x1f);
            code >>= 5;
        }

    } else {
        // ASCII
        r <<= 5;
        r |= prefix;
        r <<= 5;
        r |= code;
    }
}

template<typename T>
constexpr T tt5_from_string(char const *str)
{
    auto r = T{};
    uint32_t locked_page = 0;

    auto lookahead = tt5_from_char(*str);
    while (lookahead != 0) {
        let current = lookahead;
        lookahead = tt5_from_char(*(++str));

        let current_page = current >> 30;
        let lookahead_page = lookahead >> 30;

        if (locked_page == current_page) {
            // Current code is the locked page.
            tt5_add_code(current);

        } else if (lookahead != 0 && current_page == lookahead_page) {
            // Two characters in the same page, lock the page.

            // The Lock commands are in page 2.
            if (locked_page != 2) {
                r <<= 5;
                r |= 0x1f; // S2
            }

            // Lock page.
            r <<= 5;
            r |= 0x1b + current_page; // L0, L1, L2
            locked_page = current_page;

            tt5_add_code(current);

        } else {
            // Shift the page for a single character.
            let second_shift_command =
                (locked_page == 0 && current_page == 2) ||
                (locked_page != 0 && current_page != 0);

            r <<= 5;
            r |= 0x1e + static_cast<int>(second_shift_command); // S0, S1, S2

            tt5_add_code(code);
        }
    }

    return r;
}


}

