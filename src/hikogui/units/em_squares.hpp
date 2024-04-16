// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hikothird/au.hh>

hi_export_module(hikogui.unit : em_squares);

hi_export namespace hi {
inline namespace v1 {

struct RelativeFontLengthDim : au::base_dim::BaseDimension<1712674734> {};

struct EmSquares : au::UnitImpl<au::Dimension<RelativeFontLengthDim>> {
    static constexpr const char label[] = "em";
};
constexpr auto em_square = au::SingularNameFor<EmSquares>{};
constexpr auto em_squares = au::QuantityMaker<EmSquares>{};
constexpr auto em_squares_pt = au::QuantityPointMaker<EmSquares>{};
using em_squares_d = au::Quantity<EmSquares, double>;
using em_squares_f = au::Quantity<EmSquares, float>;
using em_squares_i = au::Quantity<EmSquares, int>;

} // namespace v1
}