// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"

namespace hi::inline v1 {

struct skeleton_expression_node final : skeleton_node {
    std::unique_ptr<formula_node> expression;

    skeleton_expression_node(parse_location location, std::unique_ptr<formula_node> expression) :
        skeleton_node(std::move(location)), expression(std::move(expression))
    {
    }

    void post_process(formula_post_process_context &context) override
    {
        post_process_expression(context, *expression, location);
    }

    std::string string() const noexcept override
    {
        return std::format("<expression {}>", *expression);
    }

    datum evaluate(formula_evaluation_context &context) override
    {
        hilet tmp = evaluate_formula_without_output(context, *expression, location);
        if (tmp.is_break()) {
            throw operation_error(std::format("{}: Found #break not inside a loop statement.", location));

        } else if (tmp.is_continue()) {
            throw operation_error(std::format("{}: Found #continue not inside a loop statement.", location));

        } else {
            return {};
        }
    }
};

} // namespace hi::inline v1
