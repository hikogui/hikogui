// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include <hikothird/au.hh>

hi_export_module(hikogui.unit : pixels);

hi_export namespace hi { inline namespace v1 {
namespace unit {

struct PixelLengthDim : au::base_dim::BaseDimension<1712674722> {};

struct Pixels : au::UnitImpl<au::Dimension<PixelLengthDim>> {
    static constexpr inline const char label[] = "px";
};
constexpr auto pixel = au::SingularNameFor<Pixels>{};
constexpr auto pixels = au::QuantityMaker<Pixels>{};
constexpr auto pixels_pt = au::QuantityPointMaker<Pixels>{};
using pixels_d = au::Quantity<Pixels, double>;
using pixels_f = au::Quantity<Pixels, float>;
using pixels_i = au::Quantity<Pixels, int>;

}}}
