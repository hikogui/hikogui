// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"

namespace hi {
inline namespace v1 {

struct theme_length : std::variant<pixels, points, em_quads> {
    using super = std::variant<pixels, points, em_quads>;
    using super::super;

    [[nodiscard]] constexpr pixels to_pixels(double pt_to_px_scale, double em_to_px_scale) const noexcept
    {
        switch (index()) {
        case 0:
            return std::get<pixels>(*this);
        case 1:
            return pixels{std::get<points>(*this).count() * pt_to_px_scale};
        case 2:
            return pixels{std::get<em_quads>(*this).count() * em_to_px_scale};
        default:
            hi_no_default();
        }
    }
};

}}
