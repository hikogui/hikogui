// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_binary_operator_node.hpp"
#include "formula_post_process_context.hpp"
#include "formula_name_node.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.formula.formula_filter_node);

hi_export namespace hi { inline namespace v1 {

hi_export struct formula_filter_node final : formula_binary_operator_node {
    mutable formula_post_process_context::filter_type filter;
    formula_name_node *rhs_name;

    formula_filter_node(size_t line_nr, size_t column_nr, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_binary_operator_node(line_nr, column_nr, std::move(lhs), std::move(rhs))
    {
        rhs_name = dynamic_cast<formula_name_node *>(this->rhs.get());
        if (rhs_name == nullptr) {
            throw parse_error(std::format("{}:{}: Expecting a name token on the right hand side of a filter operator. got {}.", line_nr, column_nr, *rhs));
        }
    }

    void post_process(formula_post_process_context &context) override
    {
        formula_binary_operator_node::post_process(context);

        filter = context.get_filter(rhs_name->name);
        if (!filter) {
            throw parse_error(std::format("{}:{}: Could not find filter .{}().", line_nr, column_nr, rhs_name->name));
        }
    }

    datum evaluate(formula_evaluation_context &context) const override
    {
        auto lhs_ = lhs->evaluate(context);
        try {
            return datum{filter(static_cast<std::string>(lhs_))};
        } catch (std::exception const &e) {
            throw operation_error(std::format("{}:{}: Can not evaluate filter.\n{}", line_nr, column_nr, e.what()));
        }
    }

    std::string string() const noexcept override
    {
        return std::format("({} ! {})", *lhs, *rhs);
    }
};

}} // namespace hi::inline v1
