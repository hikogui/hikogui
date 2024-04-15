// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hikothird/au.hh>

hi_export_module(hikogui.unit : points);

hi_export namespace hi { inline namespace v1 {

struct Points : decltype(au::Inches{} / au::mag<72>()) {
    static constexpr const char label[] = "pt";
};
constexpr auto point = au::SingularNameFor<Points>{};
constexpr auto points = au::QuantityMaker<Points>{};
constexpr auto points_pt = au::QuantityPointMaker<Points>{};
using points_d = au::Quantity<Points, double>;
using points_f = au::Quantity<Points, float>;
using points_i = au::Quantity<Points, int>;
using points_s = au::Quantity<Points, short>;



}}
