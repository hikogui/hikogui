// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"

namespace hi {
inline namespace v1 {

struct theme_length : std::variant<pixels, dips, em_quads> {
    using super = std::variant<pixels, dips, em_quads>;
    using super::super;
};

}}
