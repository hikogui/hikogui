

#pragma once

namespace hi::inline v1 {

class read_char_result {
public:
    constexpr ~read_char_result() = default;
    constexpr read_char_result() noexcept = default;
    constexpr read_char_result(read_char_result const &) noexcept = default;
    constexpr read_char_result(read_char_result &&) noexcept = default;
    constexpr read_char_result &operator=(read_char_result const &) noexcept = default;
    constexpr read_char_result &operator=(read_char_result &&) noexcept = default;

    constexpr read_char_result(char32_t c, uint8_t size = 1, bool value = true) noexcept :
        _v((c << 8) | (size << 1) | value) {}

    [[nodiscard]] constexpr char32_t code_point() const noexcept
    {
        return truncate<char32_t>(_v >> 8);
    }

    [[nodiscard]] constexpr uint8_t size() const noexcept
    {
        return truncate<uint8_t>((_v >> 1) & 0x7f);
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _v == 0;
    }

    [[nodiscard]] constexpr char32_t operator*() const noexcept
    {
        return character();
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodicard]] constexpr bool valid() const noexcept
    {
        return not to_bool(_v & 1);
    }

    [[nodiscard]] read_char_result make_invalid() const noexcept
    {
        auto r = *this;
        r._v |= 1;
        return r;
    }

private:
    // [31:8] Unicode code-point
    // [7:1] original encoding length;
    // [0] original invalid encoding.
    uint32_t _v = 0;

    [[nodiscard]] constexpr bool operator==(read_char_result const &, read_char_result const &) noexcept = default;
    [[nodiscard]] constexpr auto operator<=>(read_char_result const &, read_char_result const &) noexcept = default;
};

}

