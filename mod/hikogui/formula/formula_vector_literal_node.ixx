// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_formula_formula_vector_literal_node;
import hikogui_formula_formula_node;

export namespace hi { inline namespace v1 {

export struct formula_vector_literal_node final : formula_node {
    formula_vector values;

    formula_vector_literal_node(size_t line_nr, size_t column_nr, formula_vector values) :
        formula_node(line_nr, column_nr), values(std::move(values))
    {
    }

    void post_process(formula_post_process_context &context) override
    {
        for (auto &value : values) {
            value->post_process(context);
        }
    }

    datum evaluate(formula_evaluation_context &context) const override
    {
        datum::vector_type r;
        for (hilet &value : values) {
            r.push_back(value->evaluate(context));
        }
        return datum{std::move(r)};
    }

    datum &assign(formula_evaluation_context &context, datum const &rhs) const override
    {
        if (!holds_alternative<datum::vector_type>(rhs)) {
            throw operation_error(std::format("{}:{}: Unpacking values can only be done on vectors, got {}.", line_nr, column_nr, rhs));
        }
        if (values.size() < 1) {
            throw operation_error(std::format("{}:{}: Unpacking can only be done on 1 or more return values.", line_nr, column_nr));
        }
        if (values.size() != rhs.size()) {
            throw operation_error(std::format(
                "{}:{}: Unpacking values can only be done on with a vector of size {} got {}.",
                line_nr, column_nr,
                values.size(),
                rhs.size()));
        }

        // Make a copy, in case of self assignment.
        hilet rhs_copy = rhs;

        std::size_t i = 0;
        while (true) {
            hilet &lhs_ = values[i];
            hilet &rhs_ = rhs_copy[i];

            if (++i < rhs.size()) {
                lhs_->assign(context, rhs_);
            } else {
                return lhs_->assign(context, rhs_);
            }
        }
    }

    std::string string() const noexcept override
    {
        std::string r = "[";
        int i = 0;
        for (hilet &value : values) {
            if (i++ > 0) {
                r += ", ";
            }
            r += value->string();
        }
        return r + "]";
    }
};

}} // namespace hi::inline v1
