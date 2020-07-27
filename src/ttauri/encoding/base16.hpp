


namespace tt {

[[nodiscard]] constexpr std::pair<char,char> encode_base16(std::byte value) noexcept
{
    ttlet _lo = static_cast<char>(static_cast<uint8_t>(value) & 0xf);
    ttlet lo = _lo <= 9 ? _lo + '0' : _lo - 10 + 'A';

    ttlet _hi = static_cast<char>(static_cast<uint8_t>(value) >> 4);
    ttlet hi = _hi <= 9 ? _hi + '0' : _hi - 10 + 'A';

    return {hi, lo};
}

[[nodiscard]] inline std::string encode_base16(std::byte const *ptr, std::byte const *last) noexcept
{
    auto r = std::string{};
    r.reserve(last - ptr);

    while (ptr != last) {
        ttlet [hi, lo] = encode_base(*(ptr++));
        r += hi;
        r += lo;
    }

    return r;
}

[[nodiscard]] inline std::string encode_base16(nonstd::span<std::byte const> str) noexcept
{
    ttlet first = str.data();
    ttlet last = first + str.size();
    return encode_base16(first, last);
}


[[nodiscard]] inline std::string encode_base16(gstring const &str) noexcept
{
    ttlet first = str.data();
    ttlet last = first + str.size();
    return encode_base16(first, last);
}

[[nodiscard]] inline std::string encode_base16(gstring_view str) noexcept
{
    ttlet first = str.data();
    ttlet last = first + str.size();
    return encode_base16(first, last);
}




