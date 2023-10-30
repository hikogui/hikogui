// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_formula_formula_logical_not_node;
import hikogui_formula_formula_unary_operator_node;

export namespace hi { inline namespace v1 {

export struct formula_logical_not_node final : formula_unary_operator_node {
    formula_logical_not_node(size_t line_nr, size_t column_nr, std::unique_ptr<formula_node> rhs) :
        formula_unary_operator_node(line_nr, column_nr, std::move(rhs))
    {
    }

    datum evaluate(formula_evaluation_context &context) const override
    {
        auto rhs_ = rhs->evaluate(context);
        try {
            return datum{!rhs_};
        } catch (std::exception const &e) {
            throw operation_error(std::format("{}:{}: Can not evaluate logical not.\n{}", line_nr, column_nr, e.what()));
        }
    }

    std::string string() const noexcept override
    {
        return std::format("(! {})", *rhs);
    }
};

}} // namespace hi::inline v1
