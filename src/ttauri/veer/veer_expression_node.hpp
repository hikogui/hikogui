// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "veer_node.hpp"

namespace tt {

struct veer_expression_node final: veer_node {
    std::unique_ptr<expression_node> expression;

    veer_expression_node(parse_location location, std::unique_ptr<expression_node> expression) :
        veer_node(std::move(location)), expression(std::move(expression)) {}

    void post_process(expression_post_process_context &context) override {
        post_process_expression(context, *expression, location);
    }

    std::string string() const noexcept override {
        return fmt::format("<expression {}>", *expression);
    }

    datum evaluate(expression_evaluation_context &context) override {
        ttlet tmp = evaluate_expression_without_output(context, *expression, location);
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
