// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil_node.hpp"

namespace tt {

struct stencil_placeholder_node final : stencil_node {
    std::unique_ptr<formula_node> expression;

    stencil_placeholder_node(parse_location location, std::unique_ptr<formula_node> expression) :
        stencil_node(std::move(location)), expression(std::move(expression))
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
        } catch (parse_error &e) {
            e.merge_location(location);
            throw;
        }
    }

    std::string string() const noexcept override
    {
        return fmt::format("<placeholder {}>", *expression);
    }

    datum evaluate(formula_evaluation_context &context) override
    {
        ttlet output_size = context.output_size();

        ttlet tmp = evaluate_expression(context, *expression, location);
        if (tmp.is_break()) {
            TTAURI_THROW(invalid_operation_error("Found #break not inside a loop statement.").set_location(location));

        } else if (tmp.is_continue()) {
            TTAURI_THROW(invalid_operation_error("Found #continue not inside a loop statement.").set_location(location));

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

} // namespace tt
