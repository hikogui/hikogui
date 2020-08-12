// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "veer_node.hpp"

namespace tt {

struct veer_while_node final: veer_node {
    statement_vector children;
    std::unique_ptr<expression_node> expression;

    veer_while_node(parse_location location, std::unique_ptr<expression_node> expression) noexcept :
        veer_node(std::move(location)), expression(std::move(expression)) {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<veer_node> x) noexcept override {
        append_child(children, std::move(x));
        return true;
    }

    void post_process(expression_post_process_context &context) override {
        if (std::ssize(children) > 0) {
            children.back()->left_align();
        }

        post_process_expression(context, *expression, location);
        for (ttlet &child: children) {
            child->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context &context) override {
        ttlet output_size = context.output_size();

        ssize_t loop_count = 0;
        while (evaluate_expression_without_output(context, *expression, location)) {
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

    std::string string() const noexcept override {
        std::string s = "<while ";
        s += to_string(*expression);
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ">";
        return s;
    }
};

}