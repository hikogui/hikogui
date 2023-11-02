// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_weight.hpp"
#include "font_style.hpp"
#include "../utility/utility.hpp"
#include "../coroutine/coroutine.hpp"
#include "../macros.hpp"
#include <coroutine>
#include <string>
#include <ostream>
#include <format>
#include <utility>
#include <cstddef>

hi_export_module(hikogui.font.font_variant);

hi_export namespace hi::inline v1 {

/** A font variant is one of 16 different fonts that can be part of a family.
 * It only contains the font-weight and if it is italic/oblique.
 *
 * monospace, serif, condensed, expanded & optical-size are all part of the font family.
 */
class font_variant {
public:
    [[nodiscard]] constexpr static size_t size() noexcept
    {
        return font_weight_metadata.size() * font_style_metadata.size();
    }

    constexpr font_variant(font_weight weight, font_style style) noexcept :
        _value(narrow_cast<uint8_t>((std::to_underlying(weight) << 1) | std::to_underlying(style)))
    {
    }

    constexpr font_variant() noexcept : font_variant(font_weight::regular, font_style::normal) {}
    constexpr font_variant(font_weight weight) noexcept : font_variant(weight, font_style::normal) {}
    constexpr font_variant(font_style style) noexcept : font_variant(font_weight::regular, style) {}

    [[nodiscard]] constexpr operator size_t() const noexcept
    {
        return std::to_underlying(style()) + std::to_underlying(weight()) * font_style_metadata.size();
    }

    [[nodiscard]] size_t hash() const noexcept
    {
        return std::hash<uint8_t>{}(_value);
    }

    constexpr font_weight weight() const noexcept
    {
        return static_cast<font_weight>(_value >> 1);
    }

    [[nodiscard]] constexpr font_style style() const noexcept
    {
        return static_cast<font_style>(_value & 1);
    }

    constexpr font_variant& set_weight(font_weight rhs) noexcept
    {
        _value &= 1;
        _value |= std::to_underlying(rhs) << 1;
        return *this;
    }

    constexpr font_variant& set_style(font_style rhs) noexcept
    {
        _value &= 0x1e;
        _value |= std::to_underlying(rhs);
        return *this;
    }

    /** Get an alternative font variant.
     *
     * @param start The start of the alternative search for a font_variant.
     * @return Generated font-variants starting with @a start then zig-zag
     *         through weights, followed by zig-zag through styles.
     */
    [[nodiscard]] friend generator<font_variant> alternatives(font_variant start) noexcept
    {
        for (hilet s : alternatives(start.style())) {
            for (hilet w : alternatives(start.weight())) {
                co_yield font_variant{w, s};
            }
        }
    }

    [[nodiscard]] friend std::string to_string(font_variant const& rhs) noexcept
    {
        return std::format("{}", rhs.weight(), rhs.style() == font_style::italic ? "/italic" : "");
    }

    friend std::ostream& operator<<(std::ostream& lhs, font_variant const& rhs)
    {
        return lhs << to_string(rhs);
    }

private:
    /** The weight and style compressed in a single value.
    *
    * - [0:0] 2 different font-styles
    * - [5:1] 10 different font-weights.
    */
    uint8_t _value;
};

} // namespace hi::inline v1

hi_export template<>
struct std::hash<hi::font_variant> {
    [[nodiscard]] size_t operator()(hi::font_variant const& rhs) const noexcept
    {
        return rhs.hash();
    }
};
