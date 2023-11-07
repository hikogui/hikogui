// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <concepts>

export module hikogui_font_otype_utilities;
import hikogui_parser;
import hikogui_utility;

export namespace hi { inline namespace v1 {

/** Open-type 16.16 signed fixed point, range between -32768.0 and 32767.999
 */
struct otype_fixed15_16_buf_t {
    big_uint32_buf_t x;

    constexpr float operator*() const noexcept
    {
        return static_cast<float>(*x) / 65536.0f;
    }
};

/** Open-type 16-bit signed fraction, range between -2.0 and 1.999
 */
struct otype_fixed1_14_buf_t {
    big_int16_buf_t x;
    
    constexpr float operator*() const noexcept
    {
        return static_cast<float>(*x) / 16384.0f;
    }
};

/** Open-type for 16 signed integer that must be scaled by the EM-scale.
 */
struct otype_fword_buf_t {
    big_int16_buf_t x;
    [[nodiscard]] constexpr float operator*(float Em_scale) const noexcept
    {
        return static_cast<float>(*x) * Em_scale;
    }
};

/** Open-type for 8 signed integer that must be scaled by the EM-scale.
 */
struct otype_fbyte_buf_t {
    int8_t x;
    [[nodiscard]] constexpr float operator*(float Em_scale) const noexcept
    {
        return static_cast<float>(x) * Em_scale;
    }
};

/** Open-type for 16 unsigned integer that must be scaled by the EM-scale.
 */
struct otype_fuword_buf_t {
    big_uint16_buf_t x;
    [[nodiscard]] constexpr float operator*(float Em_scale) const noexcept
    {
        return static_cast<float>(*x) * Em_scale;
    }
};

std::optional<std::string>
otype_get_string(std::span<std::byte const> bytes, uint16_t platform_id, uint16_t platform_specific_id)
{
    switch (platform_id) {
    case 2: // Deprecated, but compatible with unicode.
    case 0: // Unicode, encoded as UTF-16BE, should not be UTF-16LE, but sometimes it is.
        hi_check(bytes.size() % 2 == 0, "Length in bytes of a name must be multiple of two");
        return char_converter<"utf-16", "utf-8">{}.read(bytes.data(), bytes.size(), std::endian::big);

    case 1: // Macintosh
        if (platform_specific_id == 0) { // Roman script ASCII
            return char_converter<"utf-8", "utf-8">{}.read(bytes.data(), bytes.size());
        }
        break;

    case 3: // Windows
        if (platform_specific_id == 1 or platform_specific_id == 10) { // UTF-16BE
            hi_check(bytes.size() % 2 == 0, "Length in bytes of a name must be multiple of two");
            return char_converter<"utf-16", "utf-8">{}.read(bytes.data(), bytes.size(), std::endian::big);
        }
        break;

    default:
        break;
    }
    return {};
}

}} // namespace hi::v1
