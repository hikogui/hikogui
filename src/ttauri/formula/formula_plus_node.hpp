// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_unary_operator_node.hpp"

namespace tt {

struct formula_plus_node final : formula_unary_operator_node {
    formula_plus_node(parse_location location, std::unique_ptr<formula_node> rhs) :
        formula_unary_operator_node(std::move(location), std::move(rhs)) {}

    datum evaluate(formula_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        try {
            return +rhs_;
        } catch (std::exception const &e) {
            throw operation_error("{}: Can not evaluate unary-plus.\n{}", location, e.what());
        }
    }

    std::string string() const noexcept override {
        return fmt::format("(+ {})", *rhs);
    }
};

}
