// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../lean_vector.hpp"
#include "../cast.hpp"
#include <cstdint>

namespace hi { inline namespace v1 {

/** 2D constraints.
 *
 * This type holds multiple possible sizes that an 2D object may be.
 * We need multiple sizes in case there is a non-linear relation between the width and height of an object.
 *
 */
class constraint2D {
public:
    struct extent_type {
        /** Width in pixels.
         */
        uint16_t width = 0;

        /** Height in pixels.
         */
        uint16_t height = 0;

        extent_type(float width, float height) noexcept :
            width(saturate_cast<uint16_t>(std::ceil(width))),
            height(saturate_cast<uint16_t>(std::ceil(height)))
        {
        }
    };

    constexpr constraint2D() noexcept = default;
    constexpr constraint2D(constraint2D const&) noexcept = default;
    constexpr constraint2D(constraint2D&&) noexcept = default;
    constexpr constraint2D& operator=(constraint2D const&) noexcept = default;
    constexpr constraint2D& operator=(constraint2D&&) noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _sizes.empty();
    }

    [[nodiscard]] extent_type const& back() const noexcept
    {
        hi_axiom(not _sizes.empty());
        return _sizes.back();
    }

    [[nodiscard]] extent_type& back() noexcept
    {
        hi_axiom(not _sizes.empty());
        return _sizes.back();
    }

    extent_type& emplace_back(float width, float height) noexcept
    {
        return _sizes.emplace_back(width, height);
    }

    constexpr void set_baseline(vertical_alignment alignment, float padding_bottom = 0.0f, float padding_top = 0.0f) noexcept
    {
        _base_line_padding_bottom = saturate_cast<uint8_t>(std::ceil(padding_bottom));
        _base_line_padding_top = saturate_cast<uint8_t>(std::ceil(padding_top));
        switch (alignment) {
        case vertical_alignment::none: _base_line_mode = 0; break;
        case vertical_alignment::bottom: _base_line_mode = 1; break;
        case vertical_alignment::top: _base_line_mode = 2; break;
        case vertical_alignment::middle: _base_line_mode = 3; break;
        }
    }

private:
    lean_vector<extent_type> _sizes = {};

    uint8_t _decimal_line_padding_left = 0;
    uint8_t _decimal_line_padding_right = 0;
    uint8_t _base_line_padding_bottom = 0;
    uint8_t _base_line_padding_top = 0;

    uint8_t _margin_left = 0;
    uint8_t _margin_right = 0;
    uint8_t _margin_bottom = 0;
    uint8_t _margin_top = 0;

    /** Mode for the decimal-line.
     *
     * Here are the modes:
     *  - 0: No decimal-line
     *  - 1: Decimal-line is on left.
     *  - 2: Decimal-line is on right.
     *  - 3: Decimal-line is in center
     */
    uint8_t _decimal_line_mode : 2 = 0;

    /** Mode for the base-line.
     *
     * Here are the modes:
     *  - 0: No base-line
     *  - 1: Base-line is at bottom.
     *  - 2: Base-line is at (top - x-height)
     *  - 3: Base-line is at (middle - 0.5 * x-height)
     *
     * In these calculations use the layout height for the widget,
     * then apply the base-line padding.
     *
     * For example (mode 2: top):
     * ```
     * base_line = std::clamp(layout_height - x_height, base_line_padding_bottom, layout_height - base_line_padding_top);
     * ```
     *
     */
    uint8_t _base_line_mode : 2 = 0;
};

}} // namespace hi::v1
