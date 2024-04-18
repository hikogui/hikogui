// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "dips.hpp"
#include "em_squares.hpp"
#include "../macros.hpp"
#include <hikothird/au.hh>

hi_export_module(hikogui.unit : dips_per_em);

hi_export namespace hi { inline namespace v1 {

struct DipsPerEm : decltype(Dips{} / EmSquares{}) {
    static constexpr const char label[] = "px/Em";
};
constexpr auto dip_per_em = au::SingularNameFor<DipsPerEm>{};
constexpr auto dips_per_em = au::QuantityMaker<DipsPerEm>{};
constexpr auto dips_per_em_pt = au::QuantityPointMaker<DipsPerEm>{};
using dips_per_em_d = au::Quantity<DipsPerEm, double>;
using dips_per_em_f = au::Quantity<DipsPerEm, float>;
using dips_per_em_i = au::Quantity<DipsPerEm, int>;
using dips_per_em_s = au::Quantity<DipsPerEm, short>;

}}
