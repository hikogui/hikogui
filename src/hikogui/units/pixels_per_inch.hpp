// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pixels.hpp"
#include "../macros.hpp"
#include <hikothird/au.hh>

hi_export_module(hikogui.unit : pixels_per_inch);

hi_export namespace hi { inline namespace v1 {

struct PixelsPerInch : decltype(Pixels{} / au::Inches{}) {
    static constexpr const char label[] = "ppi";
};
constexpr auto pixel_per_inch = au::SingularNameFor<PixelsPerInch>{};
constexpr auto pixels_per_inch = au::QuantityMaker<PixelsPerInch>{};
constexpr auto pixels_per_inch_pt = au::QuantityPointMaker<PixelsPerInch>{};
using pixels_per_inch_d = au::Quantity<PixelsPerInch, double>;
using pixels_per_inch_f = au::Quantity<PixelsPerInch, float>;
using pixels_per_inch_i = au::Quantity<PixelsPerInch, int>;

}}
