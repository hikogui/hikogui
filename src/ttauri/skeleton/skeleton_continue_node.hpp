// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "skeleton_node.hpp"

namespace tt {

struct skeleton_continue_node final: skeleton_node {
    skeleton_continue_node(parse_location location) noexcept : skeleton_node(std::move(location)) {}

    datum evaluate(formula_evaluation_context &context) override {
        return datum::_continue{};
    }

    std::string string() const noexcept override {
        return "<continue>";
    }
};

}
