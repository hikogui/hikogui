// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "points.hpp"
#include "em_squares.hpp"
#include "../macros.hpp"
#include <hikothird/au.hh>

hi_export_module(hikogui.unit : points_per_em);

hi_export namespace hi { inline namespace v1 {

struct PointsPerEm : decltype(Points{} / EmSquares{}) {
    static constexpr const char label[] = "px/Em";
};
constexpr auto point_per_em = au::SingularNameFor<PointsPerEm>{};
constexpr auto points_per_em = au::QuantityMaker<PointsPerEm>{};
constexpr auto points_per_em_pt = au::QuantityPointMaker<PointsPerEm>{};
using points_per_em_d = au::Quantity<PointsPerEm, double>;
using points_per_em_f = au::Quantity<PointsPerEm, float>;
using points_per_em_i = au::Quantity<PointsPerEm, int>;
using points_per_em_s = au::Quantity<PointsPerEm, short>;

}}
