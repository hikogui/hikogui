// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil_node.hpp"

namespace tt {

struct stencil_do_node final: stencil_node {
    statement_vector children;
    std::unique_ptr<formula_node> expression;
    parse_location formula_location;

    stencil_do_node(parse_location location) noexcept :
        stencil_node(std::move(location)) {}


    bool found_while(parse_location _location, std::unique_ptr<formula_node> x) noexcept override {
        if (expression) {
            return false;
        } else {
            expression = std::move(x);
            formula_location = std::move(_location);
            return true;
        }
    }

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<stencil_node> x) noexcept override {
        if (expression) {
            return false;
        } else {
            append_child(children, std::move(x));
            return true;
        }
    }

    void post_process(formula_post_process_context &context) override {
        if (std::ssize(children) > 0) {
            children.back()->left_align();
        }

        post_process_expression(context, *expression, location);

        for (ttlet &child: children) {
            child->post_process(context);
        }
    }

    datum evaluate(formula_evaluation_context &context) override {
        ttlet output_size = context.output_size();

        ssize_t loop_count = 0;
        do {
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

        } while (evaluate_formula_without_output(context, *expression, formula_location));
        return {};
    }

    std::string string() const noexcept override {
        tt_assert(expression);
        std::string s = "<do ";
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += to_string(*expression);
        s += ">";
        return s;
    }
};

}