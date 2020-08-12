// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "veer_node.hpp"

namespace tt {

struct veer_top_node final: veer_node {
    statement_vector children;

    veer_top_node(parse_location location) :
        veer_node(std::move(location)), children() {}

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

        for (ttlet &child: children) {
            child->post_process(context);
        }
    }

    datum evaluate(expression_evaluation_context &context) override {
        try {
            return evaluate_children(context, children);

        } catch (error &e) {
            e.merge_location(location);
            throw;
        }
    }

    std::string string() const noexcept override {
        ttlet children_str = transform<std::vector<std::string>>(children, [](ttlet &x) { return x->string(); });
        return fmt::format("<top {}>", join(children_str));
    }
};

}
