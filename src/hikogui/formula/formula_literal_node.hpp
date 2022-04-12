// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_node.hpp"

namespace hi::inline v1 {

struct formula_literal_node final : formula_node {
    datum value;

    formula_literal_node(parse_location location, datum const &value) : formula_node(std::move(location)), value(value) {}

    datum evaluate(formula_evaluation_context &context) const override
    {
        return value;
    }

    std::string string() const noexcept override
    {
        return repr(value);
    }
};

} // namespace hi::inline v1