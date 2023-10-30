// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_skeleton : return_node;
import : node;

export namespace hi::inline v1 {

struct skeleton_return_node final : skeleton_node {
    std::unique_ptr<formula_node> expression;

    skeleton_return_node(parse_location location, std::unique_ptr<formula_node> expression) noexcept :
        skeleton_node(std::move(location)), expression(std::move(expression))
    {
    }

    void post_process(formula_post_process_context &context) override
    {
        post_process_expression(context, *expression, location);
    }

    datum evaluate(formula_evaluation_context &context) override
    {
        return evaluate_formula_without_output(context, *expression, location);
    }

    std::string string() const noexcept override
    {
        return std::format("<return {}>", *expression);
    }
};

} // namespace hi::inline v1
