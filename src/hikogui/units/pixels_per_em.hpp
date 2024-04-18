// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pixels.hpp"
#include "em_squares.hpp"
#include "../macros.hpp"
#include <hikothird/au.hh>

hi_export_module(hikogui.unit : pixels_per_em);

hi_export namespace hi { inline namespace v1 {

struct PixelsPerEm : decltype(Pixels{} / EmSquares{}) {
    static constexpr const char label[] = "px/Em";
};
constexpr auto pixel_per_em = au::SingularNameFor<PixelsPerEm>{};
constexpr auto pixels_per_em = au::QuantityMaker<PixelsPerEm>{};
constexpr auto pixels_per_em_pt = au::QuantityPointMaker<PixelsPerEm>{};
using pixels_per_em_d = au::Quantity<PixelsPerEm, double>;
using pixels_per_em_f = au::Quantity<PixelsPerEm, float>;
using pixels_per_em_i = au::Quantity<PixelsPerEm, int>;
using pixels_per_em_s = au::Quantity<PixelsPerEm, short>;

}}
