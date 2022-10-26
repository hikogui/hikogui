// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_node.hpp"

namespace hi::inline v1 {

struct formula_unary_operator_node : formula_node {
    std::unique_ptr<formula_node> rhs;

    formula_unary_operator_node(parse_location location, std::unique_ptr<formula_node> rhs) :
        formula_node(std::move(location)), rhs(std::move(rhs))
    {
    }

    void post_process(formula_post_process_context &context) override
    {
        rhs->post_process(context);
    }

    std::string string() const noexcept override
    {
        return std::format("<unary_operator {}>", *rhs);
    }
};

} // namespace hi::inline v1