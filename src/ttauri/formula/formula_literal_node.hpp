// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "formula_node.hpp"

namespace tt {

struct formula_literal_node final : formula_node {
    datum value;

    formula_literal_node(parse_location location, datum const& value) :
        formula_node(std::move(location)), value(value) {}

    datum evaluate(formula_evaluation_context& context) const override {
        return value;
    }

    std::string string() const noexcept override {
        return value.repr();
    }
};

}