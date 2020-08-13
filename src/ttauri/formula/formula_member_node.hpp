// Copyright 2019, 2020 Pokitec
// All rights reserved.

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
            TTAURI_THROW(parse_error("Expecting a name token on the right hand side of a member accessor. got {}.", rhs).set_location(location));
        }
    }

    void resolve_function_pointer(formula_post_process_context& context) override {
        method = context.get_method(rhs_name->name);
        if (!method) {
            TTAURI_THROW(parse_error("Could not find method .{}().", rhs_name->name).set_location(location));
        }
    }

    datum evaluate(formula_evaluation_context& context) const override {
        if (lhs->has_evaluate_xvalue()) {
            ttlet &lhs_ = lhs->evaluate_xvalue(context);

            if (!lhs_.contains(rhs_name->name)) {
                TTAURI_THROW(invalid_operation_error("Unknown attribute .{}", rhs_name->name).set_location(location));
            }
            try {
                return lhs_[rhs_name->name];
            } catch (error &e) {
                e.set_location(location);
                throw;
            }

        } else {
            ttlet lhs_ = lhs->evaluate(context);

            if (!lhs_.contains(rhs_name->name)) {
                TTAURI_THROW(invalid_operation_error("Unknown attribute .{}", rhs_name->name).set_location(location));
            }
            try {
                return lhs_[rhs_name->name];
            } catch (error &e) {
                e.set_location(location);
                throw;
            }
        }
    }

    datum &evaluate_lvalue(formula_evaluation_context& context) const override {
        auto &lhs_ = lhs->evaluate_lvalue(context);
        try {
            return lhs_[rhs_name->name];
        } catch (error &e) {
            e.set_location(location);
            throw;
        }
    }

    datum call(formula_evaluation_context& context, datum::vector const &arguments) const override {
        auto &lhs_ = lhs->evaluate_lvalue(context);
        try {
            return method(context, lhs_, arguments);
        } catch (error &e) {
            e.set_location(location);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} . {})", *lhs, *rhs);
    }
};

}
