// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "skeleton_node.hpp"

namespace tt {

struct skeleton_return_node final: skeleton_node {
    std::unique_ptr<formula_node> expression;

    skeleton_return_node(parse_location location, std::unique_ptr<formula_node> expression) noexcept :
        skeleton_node(std::move(location)), expression(std::move(expression)) {}

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
