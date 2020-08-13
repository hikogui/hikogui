// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil_node.hpp"

namespace tt {

struct stencil_break_node final: stencil_node {
    stencil_break_node(parse_location location) noexcept : stencil_node(std::move(location)) {}

    datum evaluate(formula_evaluation_context &context) override {
        return datum::_break{};
    }

    std::string string() const noexcept override {
        return "<break>";
    }

};

}
