// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"

namespace hi::inline v1 {

struct skeleton_break_node final : skeleton_node {
    skeleton_break_node(parse_location location) noexcept : skeleton_node(std::move(location)) {}

    datum evaluate(formula_evaluation_context &context) override
    {
        return datum::make_break();
    }

    std::string string() const noexcept override
    {
        return "<break>";
    }
};

} // namespace hi::inline v1
