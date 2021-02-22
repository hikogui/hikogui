// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_binary_operator_node.hpp"

namespace tt {

struct formula_member_node final : formula_binary_operator_node {
    mutable formula_post_process_context::method_type method;
    formula_name_node* rhs_name;

    formula_member_node(parse_location location, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_binary_operator_node(std::move(location), std::move(lhs), std::move(rhs))
    {
        rhs_name = dynamic_cast<formula_name_node*>(this->rhs.get());
        if (rhs_name == nullptr) {
            throw parse_error("{}: Expecting a name token on the right hand side of a member accessor. got {}.", location, rhs);
        }
    }

    void resolve_function_pointer(formula_post_process_context& context) override {
        method = context.get_method(rhs_name->name);
        if (!method) {
            throw parse_error("{}: Could not find method .{}().", location, rhs_name->name);
        }
    }

    datum evaluate(formula_evaluation_context& context) const override {
        if (lhs->has_evaluate_xvalue()) {
            ttlet &lhs_ = lhs->evaluate_xvalue(context);

            if (!lhs_.contains(rhs_name->name)) {
                throw operation_error("{}: Unknown attribute .{}", location, rhs_name->name);
            }
            try {
                return lhs_[rhs_name->name];
            } catch (std::exception const &e) {
                throw operation_error("{}: Can not evaluate member selection.\n{}", location, e.what());
            }

        } else {
            ttlet lhs_ = lhs->evaluate(context);

            if (!lhs_.contains(rhs_name->name)) {
                throw operation_error("{}: Unknown attribute .{}", location, rhs_name->name);
            }
            try {
                return lhs_[rhs_name->name];
            } catch (std::exception const &e) {
                throw operation_error("{}: Can not evaluate member selection.\n{}", location, e.what());
            }
        }
    }

    datum &evaluate_lvalue(formula_evaluation_context& context) const override {
        auto &lhs_ = lhs->evaluate_lvalue(context);
        try {
            return lhs_[rhs_name->name];
        } catch (std::exception const &e) {
            throw operation_error("{}: Can not evaluate member-selection.\n{}", location, e.what());
        }
    }

    datum call(formula_evaluation_context& context, datum::vector const &arguments) const override {
        auto &lhs_ = lhs->evaluate_lvalue(context);
        try {
            return method(context, lhs_, arguments);
        } catch (std::exception const &e) {
            throw operation_error("{}: Can not evaluate call-of-method.\n{}", location, e.what());
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} . {})", *lhs, *rhs);
    }
};

}
