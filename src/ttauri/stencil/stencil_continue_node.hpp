// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil_node.hpp"

namespace tt {

struct stencil_continue_node final: stencil_node {
    stencil_continue_node(parse_location location) noexcept : stencil_node(std::move(location)) {}

    datum evaluate(formula_evaluation_context &context) override {
        return datum::_continue{};
    }

    std::string string() const noexcept override {
        return "<continue>";
    }
};

}
