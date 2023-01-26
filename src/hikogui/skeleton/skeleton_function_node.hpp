// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"

namespace hi::inline v1 {

struct skeleton_function_node final : skeleton_node {
    std::string name;
    std::vector<std::string> argument_names;
    statement_vector children;

    formula_post_process_context::function_type super_function;

    skeleton_function_node(
        parse_location location,
        formula_post_process_context &context,
        formula_node const &function_declaration_expression) noexcept :
        skeleton_node(std::move(location))
    {
        auto name_and_arguments = function_declaration_expression.get_name_and_argument_names();
        hi_assert(name_and_arguments.size() >= 1);

        name = name_and_arguments[0];
        name_and_arguments.erase(name_and_arguments.begin());
        argument_names = std::move(name_and_arguments);

        super_function = context.set_function(
            name, [this, &location](formula_evaluation_context &context, datum::vector_type const &arguments) {
                try {
                    return this->evaluate_call(context, arguments);

                } catch (std::exception const &e) {
                    throw operation_error(std::format("{}: Failed during handling of function call.\n{}", location, e.what()));
                }
            });
    }

    /** Append a template-piece to the current template.
     */
    bool append(std::unique_ptr<skeleton_node> x) noexcept override
    {
        append_child(children, std::move(x));
        return true;
    }

    void post_process(formula_post_process_context &context) override
    {
        if (ssize(children) > 0) {
            children.back()->left_align();
        }

        context.push_super(super_function);
        for (hilet &child : children) {
            child->post_process(context);
        }
        context.pop_super();
    }

    datum evaluate(formula_evaluation_context &context) override
    {
        return {};
    }

    datum evaluate_call(formula_evaluation_context &context, datum::vector_type const &arguments)
    {
        context.push();
        if (ssize(argument_names) != ssize(arguments)) {
            throw operation_error(std::format(
                "{}: Invalid number of arguments to function {}() expecting {} got {}.",
                location,
                name,
                argument_names.size(),
                arguments.size()));
        }

        for (ssize_t i = 0; i != ssize(argument_names); ++i) {
            context.set(argument_names[i], arguments[i]);
        }

        hilet output_size = context.output_size();
        auto tmp = evaluate_children(context, children);
        context.pop();

        if (tmp.is_break()) {
            throw operation_error(std::format("{}: Found #break not inside a loop statement.", location));

        } else if (tmp.is_continue()) {
            throw operation_error(std::format("{}: Found #continue not inside a loop statement.", location));

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            // When a function returns, it should not have written data to the output.
            context.set_output_size(output_size);
            return tmp;
        }
    }

    std::string string() const noexcept override
    {
        std::string s = "<function ";
        s += name;
        s += "(";
        s += join(argument_names, ",");
        s += ")";
        s += join(transform<std::vector<std::string>>(children, [](auto &x) {
            return to_string(*x);
        }));
        s += ">";
        return s;
    }
};

} // namespace hi::inline v1
