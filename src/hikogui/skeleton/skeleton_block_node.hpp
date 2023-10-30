// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.skeleton : block_node);

hi_export namespace hi::inline v1 {

struct skeleton_block_node final : skeleton_node {
    std::string name;
    statement_vector children;

    formula_post_process_context::function_type function;
    formula_post_process_context::function_type super_function;

    skeleton_block_node(
        parse_location location,
        formula_post_process_context &context,
        formula_node const &name_expression) noexcept :
        skeleton_node(std::move(location)), name(name_expression.get_name())
    {
        name = name_expression.get_name();

        super_function =
            context.set_function(name, [&](formula_evaluation_context &context, datum::vector_type const &arguments) {
                return this->evaluate_call(context, arguments);
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

        function = context.get_function(name);
        hi_assert(function);

        context.push_super(super_function);
        for (hilet &child : children) {
            child->post_process(context);
        }
        context.pop_super();
    }

    datum evaluate(formula_evaluation_context &context) override
    {
        datum tmp;
        try {
            tmp = function(context, datum::vector_type{});

        } catch (std::exception const &e) {
            throw operation_error(std::format("{}: Could not evaluate block.\n{}", location, e.what()));
        }

        if (tmp.is_break()) {
            throw operation_error(std::format("{}: Found #break not inside a loop statement.", location));

        } else if (tmp.is_continue()) {
            throw operation_error(std::format("{}: Found #continue not inside a loop statement.", location));

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            throw operation_error(std::format("{}: Can not use a #return statement inside a #block.", location));
        }
    }

    datum evaluate_call(formula_evaluation_context &context, datum::vector_type const &arguments)
    {
        context.push();
        auto tmp = evaluate_children(context, children);
        context.pop();

        if (tmp.is_break()) {
            throw operation_error(std::format("{}: Found #break not inside a loop statement.", location));

        } else if (tmp.is_continue()) {
            throw operation_error(std::format("{}: Found #continue not inside a loop statement.", location));

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            throw operation_error(std::format("{}: Can not use a #return statement inside a #block.", location));
        }
    }

    std::string string() const noexcept override
    {
        std::string s = "<block ";
        s += name;
        s += join(transform<std::vector<std::string>>(children, [](auto &x) {
            return to_string(*x);
        }));
        s += ">";
        return s;
    }
};

} // namespace hi::inline v1
