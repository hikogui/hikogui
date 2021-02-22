// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_binary_operator_node.hpp"

namespace tt {

struct formula_inplace_mod_node final : formula_binary_operator_node {
    formula_inplace_mod_node(parse_location location, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_binary_operator_node(std::move(location), std::move(lhs), std::move(rhs)) {}

    datum evaluate(formula_evaluation_context& context) const override {
        auto rhs_ = rhs->evaluate(context);
        auto &lhs_ = lhs->evaluate_lvalue(context);

        try {
            return lhs_ %= rhs_;
        } catch (std::exception const &e) {
            throw operation_error("{}: Can not evaluate inplace_modulo.\n{}", location, e.what());
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} %= {})", *lhs, *rhs);
    }
};

}
