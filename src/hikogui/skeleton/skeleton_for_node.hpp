// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.skeleton : for_node);

hi_export namespace hi::inline v1 {

struct skeleton_for_node final : skeleton_node {
    std::unique_ptr<formula_node> name_expression;
    std::unique_ptr<formula_node> list_expression;
    bool has_else = false;
    statement_vector children;
    statement_vector else_children;

    skeleton_for_node(
        parse_location location,
        std::unique_ptr<formula_node> name_expression,
        std::unique_ptr<formula_node> list_expression) noexcept :
        skeleton_node(std::move(location)),
        name_expression(std::move(name_expression)),
        list_expression(std::move(list_expression))
    {
    }

    /** Append a template-piece to the current template.
     */
    bool append(std::unique_ptr<skeleton_node> x) noexcept override
    {
        if (has_else) {
            append_child(else_children, std::move(x));
        } else {
            append_child(children, std::move(x));
        }
        return true;
    }

    bool found_else(parse_location _location) noexcept override
    {
        if (has_else) {
            return false;
        } else {
            has_else = true;
            return true;
        }
    }

    void post_process(formula_post_process_context &context) override
    {
        if (ssize(children) > 0) {
            children.back()->left_align();
        }
        if (ssize(else_children) > 0) {
            else_children.back()->left_align();
        }

        post_process_expression(context, *name_expression, location);
        post_process_expression(context, *list_expression, location);

        for (hilet &child : children) {
            child->post_process(context);
        }
        for (hilet &child : else_children) {
            child->post_process(context);
        }
    }

    datum evaluate(formula_evaluation_context &context) override
    {
        auto list_data = evaluate_formula_without_output(context, *list_expression, location);

        if (!holds_alternative<datum::vector_type>(list_data)) {
            throw operation_error(std::format("{}: Expecting expression returns a vector, got {}", location, list_data));
        }

        hilet output_size = context.output_size();
        if (hilet loop_size = list_data.size()) {
            ssize_t loop_count = 0;
            for (hilet &item : list_data) {
                try {
                    name_expression->assign_without_output(context, item);

                } catch (std::exception const &e) {
                    throw operation_error(std::format("{}: Could not evaluate for-loop expression.\n{}", location, e.what()));
                }

                context.loop_push(loop_count++, loop_size);
                auto tmp = evaluate_children(context, children);
                context.loop_pop();

                if (tmp.is_break()) {
                    break;
                } else if (tmp.is_continue()) {
                    continue;
                } else if (!tmp.is_undefined()) {
                    context.set_output_size(output_size);
                    return tmp;
                }
            }

        } else {
            auto tmp = evaluate_children(context, else_children);
            if (tmp.is_break() || tmp.is_continue()) {
                return tmp;
            } else if (!tmp.is_undefined()) {
                context.set_output_size(output_size);
                return tmp;
            }
        }
        return {};
    }

    std::string string() const noexcept override
    {
        std::string s = "<for ";
        s += to_string(*name_expression);
        s += ": ";
        s += to_string(*list_expression);
        s += join(transform<std::vector<std::string>>(children, [](auto &x) {
            return to_string(*x);
        }));
        if (has_else) {
            s += "else ";
            s += join(transform<std::vector<std::string>>(else_children, [](auto &x) {
                return to_string(*x);
            }));
        }
        s += ">";
        return s;
    }
};

} // namespace hi::inline v1
