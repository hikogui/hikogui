// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_formula_formula_ternary_operator_node;
import hikogui_formula_formula_node;

export namespace hi { inline namespace v1 {

export struct formula_ternary_operator_node final : formula_node {
    std::unique_ptr<formula_node> lhs;
    std::unique_ptr<formula_node> rhs_true;
    std::unique_ptr<formula_node> rhs_false;

    formula_ternary_operator_node(
        size_t line_nr, size_t column_nr,
        std::unique_ptr<formula_node> lhs,
        formula_node &pair) :
        formula_node(line_nr, column_nr), lhs(std::move(lhs))
    {
        formula_arguments &pair_ = dynamic_cast<formula_arguments &>(pair);
        hi_assert(pair_.args.size() == 2);

        rhs_true = std::move(pair_.args[0]);
        rhs_false = std::move(pair_.args[1]);
        // The unique_ptrs inside pair_ are now empty.
    }

    void post_process(formula_post_process_context &context) override
    {
        lhs->post_process(context);
        rhs_true->post_process(context);
        rhs_false->post_process(context);
    }

    datum evaluate(formula_evaluation_context &context) const override
    {
        hilet lhs_ = lhs->evaluate(context);
        if (lhs_) {
            return rhs_true->evaluate(context);
        } else {
            return rhs_false->evaluate(context);
        }
    }

    std::string string() const noexcept override
    {
        return std::format("({} ? {} : {})", *lhs, *rhs_true, *rhs_false);
    }
};

}} // namespace hi::inline v1
