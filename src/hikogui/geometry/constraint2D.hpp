// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <vector>

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

        uint8_t decimal_line_padding_left = 0;
        uint8_t decimal_line_padding_right = 0;
        uint8_t base_line_padding_bottom = 0;
        uint8_t base_line_padding_top = 0;

        uint8_t margin_left = 0;
        uint8_t margin_right = 0;
        uint8_t margin_bottom = 0;
        uint8_t margin_top = 0;

        uint16_t reserved = 0;

        /** Priority used to select this extent.
         *
         * Higher value is higher priority.
         */
        uint8_t priority = 0;

        /** This extent is used as the maximum size of the widget.
         */
        uint8_t maximum : 1 = 0;

        /** Mode for the decimal-line.
         *
         * Here are the modes:
         *  - 0: No decimal-line
         *  - 1: Decimal-line is on left.
         *  - 2: Decimal-line is on right.
         *  - 3: Decimal-line is in center
         */
        uint8_t decimal_line_mode : 2 = 0;

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
        uint8_t base_line_mode : 2 = 0;

        constexpr void set_base_line(vertical_alignment alignment) noexcept
        {
            switch (alignment) {
            case vertical_alignment::bottom:
                base_line_mode = 1;
                break;
            case vertical_alignment::top:
                base_line_mode = 2;
                break;
            case vertical_alignment::middle:
                base_line_mode = 3;
                break;
            }
        }

        [[nodiscard]] constexpr bool holds_invariant() const noexcept
        {
            if (base_line_mode == 0) {
                if (base_line_padding_bottom != 0 or base_line_padding_top != 0) {
                    return false;
                }
            } else {
                if (base_line_padding_bottom + base_line_padding_top > height) {
                    return false;
                }
            }

            if (decimal_line_mode == 0) {
                if (decimal_line_padding_left != 0 or decimal_line_padding_right != 0) {
                    return false;
                }
            } else {
                if (decimal_line_padding_left + decimal_line_padding_right > width) {
                    return false;
                }
            }

            if (reserved != 0) {
                return false;
            }
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

    [[nodiscard]] constexpr extent_type const& back() const noexcept
    {
        hi_axiom(not _sizes.empty());
        return _sizes.back();
    }

    [[nodiscard]] constexpr extent_type& back() noexcept
    {
        hi_axiom(not _sizes.empty());
        return _sizes.back();
    }

    constexpr void emplace_back(auto&&...args) noexcept
    {
        return _sizes.emplace_back(hi_forward(args)...);
    }

private:
    std::vector<extent_type> _sizes;
};

}} // namespace hi::v1
