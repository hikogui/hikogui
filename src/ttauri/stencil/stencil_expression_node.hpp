// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil_node.hpp"

namespace tt {

struct stencil_expression_node final: stencil_node {
    std::unique_ptr<formula_node> expression;

    stencil_expression_node(parse_location location, std::unique_ptr<formula_node> expression) :
        stencil_node(std::move(location)), expression(std::move(expression)) {}

    void post_process(formula_post_process_context &context) override {
        post_process_expression(context, *expression, location);
    }

    std::string string() const noexcept override {
        return fmt::format("<expression {}>", *expression);
    }

    datum evaluate(formula_evaluation_context &context) override {
        ttlet tmp = evaluate_formula_without_output(context, *expression, location);
        if (tmp.is_break()) {
            TTAURI_THROW(invalid_operation_error("Found #break not inside a loop statement.").set_location(location));

        } else if (tmp.is_continue()) {
            TTAURI_THROW(invalid_operation_error("Found #continue not inside a loop statement.").set_location(location));

        } else {
            return {};
        }
    }
};

}
