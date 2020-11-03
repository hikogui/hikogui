// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "skeleton_node.hpp"

namespace tt {

struct skeleton_top_node final: skeleton_node {
    statement_vector children;

    skeleton_top_node(parse_location location) :
        skeleton_node(std::move(location)), children() {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<skeleton_node> x) noexcept override {
        append_child(children, std::move(x));
        return true;
    }

    void post_process(formula_post_process_context &context) override {
        if (std::ssize(children) > 0) {
            children.back()->left_align();
        }

        for (ttlet &child: children) {
            child->post_process(context);
        }
    }

    datum evaluate(formula_evaluation_context &context) override {
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
