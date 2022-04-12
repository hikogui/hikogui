// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"

namespace hi::inline v1 {

struct skeleton_placeholder_node final : skeleton_node {
    std::unique_ptr<formula_node> expression;

    skeleton_placeholder_node(parse_location location, std::unique_ptr<formula_node> expression) :
        skeleton_node(std::move(location)), expression(std::move(expression))
    {
    }

    [[nodiscard]] bool should_left_align() const noexcept override
    {
        return false;
    }

    void post_process(formula_post_process_context &context) override
    {
        try {
            expression->post_process(context);

        } catch (std::exception const &e) {
            throw operation_error(std::format("{}: Could not post process placeholder.\n{}", location, e.what()));
        }
    }

    std::string string() const noexcept override
    {
        return std::format("<placeholder {}>", *expression);
    }

    datum evaluate(formula_evaluation_context &context) override
    {
        hilet output_size = context.output_size();

        hilet tmp = evaluate_expression(context, *expression, location);
        if (tmp.is_break()) {
            throw operation_error(std::format("{}: Found #break not inside a loop statement.", location));

        } else if (tmp.is_continue()) {
            throw operation_error(std::format("{}: Found #continue not inside a loop statement.", location));

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            // When a function returns, it should not have written data to the output.
            context.set_output_size(output_size);
            context.write(static_cast<std::string>(tmp));
            return {};
        }
    }
};

} // namespace hi::inline v1
