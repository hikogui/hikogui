// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.skeleton : continue_node);

hi_export namespace hi::inline v1 {

struct skeleton_continue_node final : skeleton_node {
    skeleton_continue_node(parse_location location) noexcept : skeleton_node(std::move(location)) {}

    datum evaluate(formula_evaluation_context &context) override
    {
        return datum::make_continue();
    }

    std::string string() const noexcept override
    {
        return "<continue>";
    }
};

} // namespace hi::inline v1
