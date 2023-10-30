// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_formula_formula_literal_node;
import hikogui_formula_formula_node;

export namespace hi { inline namespace v1 {

export struct formula_literal_node final : formula_node {
    datum value;

    formula_literal_node(size_t line_nr, size_t column_nr, datum const &value) : formula_node(line_nr, column_nr), value(value) {}

    datum evaluate(formula_evaluation_context &context) const override
    {
        return value;
    }

    std::string string() const noexcept override
    {
        return repr(value);
    }
};

}} // namespace hi::inline v1
