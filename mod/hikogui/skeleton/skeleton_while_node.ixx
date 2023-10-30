// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_skeleton : while_node;
import : node;

export namespace hi::inline v1 {

struct skeleton_while_node final : skeleton_node {
    statement_vector children;
    std::unique_ptr<formula_node> expression;

    skeleton_while_node(parse_location location, std::unique_ptr<formula_node> expression) noexcept :
        skeleton_node(std::move(location)), expression(std::move(expression))
    {
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

        post_process_expression(context, *expression, location);
        for (hilet &child : children) {
            child->post_process(context);
        }
    }

    datum evaluate(formula_evaluation_context &context) override
    {
        hilet output_size = context.output_size();

        ssize_t loop_count = 0;
        while (evaluate_formula_without_output(context, *expression, location)) {
            context.loop_push(loop_count++);
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
        return {};
    }

    std::string string() const noexcept override
    {
        std::string s = "<while ";
        s += to_string(*expression);
        s += join(transform<std::vector<std::string>>(children, [](auto &x) {
            return to_string(*x);
        }));
        s += ">";
        return s;
    }
};

} // namespace hi::inline v1
