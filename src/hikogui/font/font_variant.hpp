// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_weight.hpp"

namespace hi::inline v1 {

/** A font variant is one of 16 different fonts that can be part of a family.
 * It only contains the font-weight and if it is italic/oblique.
 *
 * monospace, serif, condensed, expanded & optical-size are all part of the font family.
 */
class font_variant {
    uint8_t value;

public:
    constexpr static int max()
    {
        return 20;
    }
    constexpr static int half()
    {
        return max() / 2;
    }

    constexpr font_variant(font_weight weight, bool italic) noexcept :
        value(static_cast<uint8_t>(static_cast<int>(weight) + (italic ? half() : 0)))
    {
    }
    constexpr font_variant() noexcept : font_variant(font_weight::Regular, false) {}
    constexpr font_variant(font_weight weight) noexcept : font_variant(weight, false) {}
    constexpr font_variant(bool italic) noexcept : font_variant(font_weight::Regular, italic) {}

    [[nodiscard]] size_t hash() const noexcept
    {
        return std::hash<uint8_t>{}(value);
    }

    constexpr font_weight weight() const noexcept
    {
        hi_axiom(value < max());
        return static_cast<font_weight>(value % half());
    }

    [[nodiscard]] constexpr bool italic() const noexcept
    {
        hi_axiom(value < max());
        return value >= half();
    }

    constexpr font_variant &set_weight(font_weight rhs) noexcept
    {
        value = static_cast<uint8_t>(static_cast<int>(rhs) + (italic() ? half() : 0));
        hi_axiom(value < max());
        return *this;
    }

    constexpr font_variant &set_italic(bool rhs) noexcept
    {
        value = static_cast<uint8_t>(static_cast<int>(weight()) + (rhs ? half() : 0));
        hi_axiom(value < max());
        return *this;
    }

    constexpr operator int() const noexcept
    {
        hi_axiom(value < max());
        return value;
    }

    /** Get an alternative font variant.
     * @param i 0 is current value, 1 is best alternative, 15 is worst alternative.
     */
    constexpr font_variant alternative(int i) const noexcept
    {
        hi_axiom(i >= 0 && i < max());
        hilet w = font_weight_alterative(weight(), i % half());
        hilet it = italic() == (i < half());
        return {w, it};
    }

    [[nodiscard]] friend std::string to_string(font_variant const &rhs) noexcept
    {
        return std::format("{}", rhs.weight(), rhs.italic() ? "/italic" : "");
    }

    friend std::ostream &operator<<(std::ostream &lhs, font_variant const &rhs)
    {
        return lhs << to_string(rhs);
    }
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::font_variant> {
    [[nodiscard]] size_t operator()(hi::font_variant const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
