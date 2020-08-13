// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "formula_binary_operator_node.hpp"

namespace tt {

struct formula_inplace_shr_node final : formula_binary_operator_node {
    formula_inplace_shr_node(parse_location location, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_binary_operator_node(std::move(location), std::move(lhs), std::move(rhs)) {}

    datum evaluate(formula_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ >>= rhs_;
        } catch (error &e) {
            e.set_location(location);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} >>= {})", *lhs, *rhs);
    }
};

}
