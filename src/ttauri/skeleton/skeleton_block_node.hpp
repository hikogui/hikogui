// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"

namespace tt {

struct skeleton_block_node final: skeleton_node {
    std::string name;
    statement_vector children;

    formula_post_process_context::function_type function;
    formula_post_process_context::function_type super_function;

    skeleton_block_node(parse_location location, formula_post_process_context &context, std::unique_ptr<formula_node> name_expression) noexcept :
        skeleton_node(std::move(location)), name(name_expression->get_name())
    {
        name = name_expression->get_name();

        super_function = context.set_function(name,
            [&](formula_evaluation_context &context, datum::vector const &arguments) {
            return this->evaluate_call(context, arguments);
        }
        );
    }

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

        function = context.get_function(name);
        tt_assert(function);

        context.push_super(super_function);
        for (ttlet &child: children) {
            child->post_process(context);
        }
        context.pop_super();
    }

    datum evaluate(formula_evaluation_context &context) override {
        datum tmp;
        try {
            tmp = function(context, datum::vector{});

        } catch (...) {
            auto error_location = location;
            if (ttlet location_in_function = error_info::peek<parse_location, "parse_location">()) {
                error_location += *location_in_function;
            }
            error_info(true).set<"parse_location">(error_location);
            throw;
        }

        if (tmp.is_break()) {
            tt_error_info().set<"parse_location">(location);
            throw operation_error("Found #break not inside a loop statement.");

        } else if (tmp.is_continue()) {
            tt_error_info().set<"parse_location">(location);
            throw operation_error("Found #continue not inside a loop statement.");

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            tt_error_info().set<"parse_location">(location);
            throw operation_error("Can not use a #return statement inside a #block.");
        }
    }

    datum evaluate_call(formula_evaluation_context &context, datum::vector const &arguments) {
        context.push();
        auto tmp = evaluate_children(context, children);
        context.pop();

        if (tmp.is_break()) {
            tt_error_info().set<"parse_location">(location);
            throw operation_error("Found #break not inside a loop statement.");

        } else if (tmp.is_continue()) {
            tt_error_info().set<"parse_location">(location);
            throw operation_error("Found #continue not inside a loop statement.");

        } else if (tmp.is_undefined()) {
            return {};

        } else {
            tt_error_info().set<"parse_location">(location);
            throw operation_error("Can not use a #return statement inside a #block.");
        }
    }

    std::string string() const noexcept override {
        std::string s = "<block ";
        s += name;
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ">";
        return s;
    }
};

}