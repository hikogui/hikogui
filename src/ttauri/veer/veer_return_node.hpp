// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "veer_node.hpp"

namespace tt {

struct veer_return_node final: veer_node {
    std::unique_ptr<expression_node> expression;

    veer_return_node(parse_location location, std::unique_ptr<expression_node> expression) noexcept :
        veer_node(std::move(location)), expression(std::move(expression)) {}

    void post_process(expression_post_process_context &context) override {
        post_process_expression(context, *expression, location);
    }

    datum evaluate(expression_evaluation_context &context) override {
        return evaluate_expression_without_output(context, *expression, location);
    }

    std::string string() const noexcept override {
        return fmt::format("<return {}>", *expression);
    }
};

}
