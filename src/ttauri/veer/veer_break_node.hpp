// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "veer_node.hpp"

namespace tt {

struct veer_break_node final: veer_node {
    veer_break_node(parse_location location) noexcept : veer_node(std::move(location)) {}

    datum evaluate(expression_evaluation_context &context) override {
        return datum::_break{};
    }

    std::string string() const noexcept override {
        return "<break>";
    }

};

}
