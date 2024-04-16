// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hikothird/au.hh>

hi_export_module(hikogui.unit : pixels_per_inch);

hi_export namespace hi { inline namespace v1 {

struct DeviceIndependentPixelLengthDim : au::base_dim::BaseDimension<1712760807> {};

/** Device Independent Pixel.
 *
 * Device Independent Pixels are scaled not only based on the
 * PPI (pixels per inch) of the display, but also based on the viewing distance.
 * This will help make text and other graphic elements equally readable on
 * mobile devices, computers and televisions.
 *
 * The scaling factor for Device Independent Pixels is rounded to improve pixel
 * alignment, example scaling factors are: 0.75, 1.0 (base), 1.5, 2.0, 3.0, 4.0.
 *
 * A Device Independent Pixel is about 1/160th of an inch on a mobile phone.
 */
struct Dips : au::UnitImpl<au::Dimension<DeviceIndependentPixelLengthDim>> {
    static constexpr inline const char label[] = "dp";
};
constexpr auto dip = au::SingularNameFor<Dips>{};
constexpr auto dips = au::QuantityMaker<Dips>{};
constexpr auto dips_pt = au::QuantityPointMaker<Dips>{};
using dips_d = au::Quantity<Dips, double>;
using dips_f = au::Quantity<Dips, float>;
using dips_i = au::Quantity<Dips, int>;

}}
