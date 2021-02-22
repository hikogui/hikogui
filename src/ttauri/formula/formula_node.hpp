// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_post_process_context.hpp"
#include "formula_evaluation_context.hpp"
#include "../required.hpp"
#include "../parse_location.hpp"
#include "../datum.hpp"
#include "../exception.hpp"
#include <vector>
#include <memory>
#include <string>

namespace tt {

struct formula_node {
    using formula_vector = std::vector<std::unique_ptr<formula_node>>;

    parse_location location;

    formula_node(parse_location location) : location(location) {}

    virtual ~formula_node() {}

    /** Resolve function and method pointers.
    * At all call-formulas resolve the function pointers from the parse_context.
    */
    virtual void post_process(formula_post_process_context& context) {}

    /** Resolve function and method pointers.
    * This is called on a name-formula or member-formula to set the function pointer.
    */
    virtual void resolve_function_pointer(formula_post_process_context& context) {}

    /** Evaluate an rvalue.
    */
    virtual datum evaluate(formula_evaluation_context& context) const = 0;

    datum evaluate_without_output(formula_evaluation_context& context) const {
        context.disable_output();
        auto r = evaluate(context);
        context.enable_output();
        return r;
    }

    /** Evaluate an existing lvalue.
    */
    virtual datum &evaluate_lvalue(formula_evaluation_context& context) const {
        throw operation_error("{}: Expression is not a modifiable value.", location);
    }

    virtual bool has_evaluate_xvalue() const {
        return false;
    }

    /** Evaluate an existing xvalue.
    */
    virtual datum const &evaluate_xvalue(formula_evaluation_context const& context) const {
        throw operation_error("{}: Expression is not a xvalue.", location);
    }

    /** Assign to a non-existing or existing lvalue.
    */
    virtual datum &assign(formula_evaluation_context& context, datum const &rhs) const {
        return evaluate_lvalue(context) = rhs;
    }

    datum &assign_without_output(formula_evaluation_context& context, datum const &rhs) const {
        context.disable_output();
        auto &r = assign(context, rhs);
        context.enable_output();
        return r;
    }

    /** Call a function with a datum::vector as arguments.
    */
    virtual datum call(formula_evaluation_context& context, datum::vector const &arguments) const {
        throw operation_error("{}: Expression is not callable.", location);
    }

    /** Get the name of a formula_name_node.
    */
    virtual std::string get_name() const {
        throw parse_error("{}: Expect a name got {})", location, *this);
    }

    /** Get name and argument names from a function declaration.
    * This is only implemented on the formula_call_node.
    */
    virtual std::vector<std::string> get_name_and_argument_names() const {
        throw parse_error("{}: Expect a function definition got {})", location, *this);
    }

    virtual std::string string() const noexcept = 0;

    friend std::string to_string(formula_node const& rhs) noexcept {
        return rhs.string();
    }

    friend std::ostream& operator<<(std::ostream& lhs, formula_node const& rhs) noexcept {
        return lhs << to_string(rhs);
    }
};

}
