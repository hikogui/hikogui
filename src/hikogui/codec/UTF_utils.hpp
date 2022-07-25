


#pragma once


namespace hi::inline v1 {


[[nodiscard]] constexpr int length_utf(char32_t src, char16_t *) noexcept
{
    hi_axiom(c <= 0x10ffff);
    return (c > 0xffff) + 1;
}

[[nodiscard]] constexpr int length_utf(char32_t src, char32_t *) noexcept
{
    hi_axiom(c <= 0x10ffff);
    return 1;
}

template<bool Write>
[[nodiscard]] constexpr uint8_t write_utf8(char32_t c, char8_t *ptr) noexcept
{
    hi_axiom(c <= 0x10'ffff);

    auto length = truncate<uint8_t>((c > 0x7f) + (c > 0x7ff) + (c > 0xffff));
    if constexpr (Write) {
        if (auto i = length) {
            do {
                ptr[i] = truncate<char8_t>((c & 0x3f) | 0x80);
                c >>= 6;
            } while (--i);

            c |= 0x780 >> length;
        }
        ptr[0] = truncate<char8_t>(c);
    }
    return length + 1;
}

[[nodiscard]] constexpr std::pair<char32_t, uint8_t> read_utf8(char8_t *ptr, size_t size) noexcept
{
    hi_axiom(size > 0);

    auto c = ptr[0];

    if (c & 0x80) {
        auto length = std::countl_ones(c);
        if (length > size) {
            return {0xfffd, size};
        }

        c &= 0x7f >> length;

        do {
            c <<= 6;
            c |= *(++ptr) & 0x3f;
        }

    } else {
        return {truncate<char32_t>(c), 1};
    }
}


template<bool Write>
[[nodiscard]] constexpr uint8_t write_utf16(char32_t c, char16_t *ptr) noexcept
{
    hi_axiom(c <= 0x10'ffff);

    if (auto tmp = truncate<int32_t>(c) - 0x1'0000; tmp >= 0) {
        if constexpr (Write) {
            ptr[1] = truncate<char16_t>((tmp & 0x3ff) + 0xdc00);
            tmp >>= 10;
            ptr[0] = truncate<char16_t>(tmp + 0xd800);
        }
        return 2;

    } else {
        if constexpr (Write) {
            ptr[0] = truncate<char16_t>(c);
        }
        return 1;
    }
}

template<bool Write>
[[nodiscard]] constexpr uint8_t write_utf32(char32_t c, char32_t *ptr) noexcept
{
    if constexpr (Write) {
        ptr[0] = c;
    }
    return 1;
}

template<bool Write, typename Output>
[[nodiscard]] constexpr int write_utf(char32_t src, Output *dst_ptr) noexcept
{
    if (src > 0x10'ffff) {
        [[unlikely]] src = 0xfffd;
    }

    if constexpr (Write) {
        return raw_write_utf(src, dst_ptr);
    } else {
        return length_utf(src, dst_ptr);
    }
}

}

