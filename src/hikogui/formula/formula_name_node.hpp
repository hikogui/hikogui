// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_node.hpp"

namespace hi::inline v1 {

struct formula_name_node final : formula_node {
    std::string name;
    mutable formula_post_process_context::function_type function;

    formula_name_node(parse_location location, std::string_view name) : formula_node(std::move(location)), name(name) {}

    void resolve_function_pointer(formula_post_process_context &context) override
    {
        function = context.get_function(name);
        if (!function) {
            throw parse_error(std::format("{}: Could not find function {}().", location, name));
        }
    }

    datum evaluate(formula_evaluation_context &context) const override
    {
        hilet &const_context = context;

        try {
            return const_context.get(name);
        } catch (std::exception const &e) {
            throw operation_error(std::format("{}: Can not evaluate function.\n{}", location, e.what()));
        }
    }

    datum &evaluate_lvalue(formula_evaluation_context &context) const override
    {
        try {
            return context.get(name);
        } catch (std::exception const &e) {
            throw operation_error(std::format("{}: Can not evaluate function.\n{}", location, e.what()));
        }
    }

    bool has_evaluate_xvalue() const override
    {
        return true;
    }

    /** Evaluate an existing xvalue.
     */
    datum const &evaluate_xvalue(formula_evaluation_context const &context) const override
    {
        try {
            return context.get(name);
        } catch (std::exception const &e) {
            throw operation_error(std::format("{}: Can not evaluate function.\n{}", location, e.what()));
        }
    }

    datum &assign(formula_evaluation_context &context, datum const &rhs) const override
    {
        try {
            return context.set(name, rhs);
        } catch (std::exception const &e) {
            throw operation_error(std::format("{}: Can not evaluate function.\n{}", location, e.what()));
        }
    }

    datum call(formula_evaluation_context &context, datum::vector_type const &arguments) const override
    {
        return function(context, arguments);
    }

    std::string get_name() const noexcept override
    {
        return name;
    }

    std::string string() const noexcept override
    {
        return name;
    }
};

} // namespace hi::inline v1