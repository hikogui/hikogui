// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include <hikothird/au.hh>

hi_export_module(hikogui.unit : img_squares);

hi_export namespace hi {
inline namespace v1 {
namespace unit {

struct RelativeImageLengthDim : au::base_dim::BaseDimension<1727731092> {};

struct ImageSquares: au::UnitImpl<au::Dimension<RelativeImageLengthDim>> {
    static constexpr const char label[] = "pct";
};
constexpr auto img_square = au::SingularNameFor<ImageSquares>{};
constexpr auto img_squares = au::QuantityMaker<ImageSquares>{};
constexpr auto img_squares_pt = au::QuantityPointMaker<ImageSquares>{};
using img_squares_d = au::Quantity<ImageSquares, double>;
using img_squares_f = au::Quantity<ImageSquares, float>;

}} // namespace v1
}
