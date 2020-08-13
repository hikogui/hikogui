// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "formula_node.hpp"

namespace tt {

struct formula_unary_operator_node : formula_node {
    std::unique_ptr<formula_node> rhs;

    formula_unary_operator_node(parse_location location, std::unique_ptr<formula_node> rhs) :
        formula_node(std::move(location)), rhs(std::move(rhs)) {}

    void post_process(formula_post_process_context& context) override {
        rhs->post_process(context);
    }

    std::string string() const noexcept override {
        return fmt::format("<unary_operator {}>", rhs);
    }
};

}