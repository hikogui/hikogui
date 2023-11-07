// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vector>
#include <memory>
#include <string>

export module hikogui_formula_formula_node;
import hikogui_codec;
import hikogui_formula_formula_evaluation_context;
import hikogui_formula_formula_post_process_context;
import hikogui_parser;
import hikogui_utility;

export namespace hi { inline namespace v1 {

struct formula_node {
    using formula_vector = std::vector<std::unique_ptr<formula_node>>;

    size_t line_nr;
    size_t column_nr;

    virtual ~formula_node() {}
    formula_node() = delete;
    formula_node(formula_node const&) = delete;
    formula_node(formula_node&&) = delete;
    formula_node& operator=(formula_node const&) = delete;
    formula_node& operator=(formula_node&&) = delete;

    formula_node(size_t line_nr, size_t column_nr) : line_nr(line_nr), column_nr(column_nr) {}

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

    datum evaluate_without_output(formula_evaluation_context& context) const
    {
        context.disable_output();
        auto r = evaluate(context);
        context.enable_output();
        return r;
    }

    /** Evaluate an existing lvalue.
     */
    virtual datum& evaluate_lvalue(formula_evaluation_context& context) const
    {
        throw operation_error(std::format("{}:{}: Expression is not a modifiable value.", line_nr, column_nr));
    }

    virtual bool has_evaluate_xvalue() const
    {
        return false;
    }

    /** Evaluate an existing xvalue.
     */
    virtual datum const& evaluate_xvalue(formula_evaluation_context const& context) const
    {
        throw operation_error(std::format("{}:{}: Expression is not a xvalue.", line_nr, column_nr));
    }

    /** Assign to a non-existing or existing lvalue.
     */
    virtual datum& assign(formula_evaluation_context& context, datum const& rhs) const
    {
        return evaluate_lvalue(context) = rhs;
    }

    datum& assign_without_output(formula_evaluation_context& context, datum const& rhs) const
    {
        context.disable_output();
        auto& r = assign(context, rhs);
        context.enable_output();
        return r;
    }

    /** Call a function with a datum::vector as arguments.
     */
    virtual datum call(formula_evaluation_context& context, datum::vector_type const& arguments) const
    {
        throw operation_error(std::format("{}:{}: Expression is not callable.", line_nr, column_nr));
    }

    /** Get the name of a formula_name_node.
     */
    virtual std::string get_name() const
    {
        throw parse_error(std::format("{}:{}: Expect a name, got {})", line_nr, column_nr, to_string(*this)));
    }

    /** Get name and argument names from a function declaration.
     * This is only implemented on the formula_call_node.
     */
    virtual std::vector<std::string> get_name_and_argument_names() const
    {
        throw parse_error(std::format("{}:{}: Expect a function definition, got {})", line_nr, column_nr, to_string(*this)));
    }

    virtual std::string string() const noexcept = 0;

    friend std::string to_string(formula_node const& rhs) noexcept
    {
        return rhs.string();
    }
};

}} // namespace hi::v1

// XXX #617 MSVC bug does not handle partial specialization in modules.
export template<>
struct std::formatter<hi::formula_node, char> : std::formatter<std::string_view, char> {
    auto format(hi::formula_node const& t, auto& fc) const
    {
        return std::formatter<std::string_view, char>::format(to_string(t), fc);
    }
};

export namespace hi { inline namespace v1 {

std::ostream& operator<<(std::ostream& lhs, formula_node const& rhs) noexcept
{
    return lhs << std::format("{}", rhs);
}

}} // namespace hi::v1
