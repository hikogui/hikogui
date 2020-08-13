// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "formula_unary_operator_node.hpp"

namespace tt {

struct formula_invert_node final : formula_unary_operator_node {
    formula_invert_node(parse_location location, std::unique_ptr<formula_node> rhs) :
        formula_unary_operator_node(std::move(location), std::move(rhs)) {}

    datum evaluate(formula_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        try {
            return ~rhs_;
        } catch (error &e) {
            e.set_location(location);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("(~ {})", *rhs);
    }
};

}
