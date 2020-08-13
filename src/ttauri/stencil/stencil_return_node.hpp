// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil_node.hpp"

namespace tt {

struct stencil_return_node final: stencil_node {
    std::unique_ptr<formula_node> expression;

    stencil_return_node(parse_location location, std::unique_ptr<formula_node> expression) noexcept :
        stencil_node(std::move(location)), expression(std::move(expression)) {}

    void post_process(formula_post_process_context &context) override {
        post_process_expression(context, *expression, location);
    }

    datum evaluate(formula_evaluation_context &context) override {
        return evaluate_formula_without_output(context, *expression, location);
    }

    std::string string() const noexcept override {
        return fmt::format("<return {}>", *expression);
    }
};

}
