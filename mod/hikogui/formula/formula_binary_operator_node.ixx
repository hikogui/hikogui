// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_formula_formula_binary_operator_node;
import hikogui_formula_formula_node;

export namespace hi { inline namespace v1 {

export struct formula_binary_operator_node : formula_node {
    std::unique_ptr<formula_node> lhs;
    std::unique_ptr<formula_node> rhs;

    formula_binary_operator_node(size_t line_nr, size_t column_nr, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_node(line_nr, column_nr), lhs(std::move(lhs)), rhs(std::move(rhs))
    {
    }

    void post_process(formula_post_process_context &context) override
    {
        lhs->post_process(context);
        rhs->post_process(context);
    }

    std::string string() const noexcept override
    {
        return std::format("<binary_operator {}, {}>", *lhs, *rhs);
    }
};

}} // namespace hi::inline v1
