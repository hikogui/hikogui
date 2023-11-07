// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_binary_operator_node.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.formula.formula_assign_node);

hi_export namespace hi { inline namespace v1 {

hi_export struct formula_assign_node final : formula_binary_operator_node {
    formula_assign_node(size_t line_nr, size_t column_nr, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_binary_operator_node(line_nr, column_nr, std::move(lhs), std::move(rhs))
    {
    }

    datum evaluate(formula_evaluation_context &context) const override
    {
        auto rhs_ = rhs->evaluate(context);
        return lhs->assign(context, rhs_);
    }

    std::string string() const noexcept override
    {
        return std::format("({} = {})", *lhs, *rhs);
    }
};

}} // namespace hi::inline v1
