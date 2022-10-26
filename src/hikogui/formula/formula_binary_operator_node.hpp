// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_node.hpp"

namespace hi::inline v1 {

struct formula_binary_operator_node : formula_node {
    std::unique_ptr<formula_node> lhs;
    std::unique_ptr<formula_node> rhs;

    formula_binary_operator_node(parse_location location, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_node(std::move(location)), lhs(std::move(lhs)), rhs(std::move(rhs))
    {
    }

    void post_process(formula_post_process_context &context) override
    {
        lhs->post_process(context);
        rhs->post_process(context);
    }

    std::string string() const noexcept override
    {
        return std::format("<binary_operator {}, {}>", *lhs, *rhs);
    }
};

} // namespace hi::inline v1