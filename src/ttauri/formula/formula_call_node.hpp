// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "formula_node.hpp"

namespace tt {

struct formula_call_node final : formula_node {
    std::unique_ptr<formula_node> lhs;
    formula_vector args;

    formula_call_node(
        parse_location location,
        std::unique_ptr<formula_node> lhs,
        std::unique_ptr<formula_node> rhs
    ) :
        formula_node(std::move(location)), lhs(std::move(lhs))
    {
        auto rhs_ = dynamic_cast<formula_arguments*>(rhs.get());
        tt_assert(rhs_ != nullptr);
        args = std::move(rhs_->args);
    }

    void post_process(formula_post_process_context& context) override {
        lhs->resolve_function_pointer(context);
        for (auto &arg: args) {
            arg->post_process(context);
        }
    }

    datum evaluate(formula_evaluation_context& context) const override {
        ttlet args_ = transform<datum::vector>(args, [&](ttlet& x) {
            return x->evaluate(context);
        });

        return lhs->call(context, args_);
    }

    std::vector<std::string> get_name_and_argument_names() const override {
        std::vector<std::string> r;

        try {
            r.push_back(lhs->get_name());
        } catch (parse_error &) {
            throw parse_error(fmt::format("Function definition does not have a name, got {})", lhs));
        }

        for (ttlet &arg: args) {
            try {
                r.push_back(arg->get_name());
            } catch (parse_error &) {
                throw parse_error(fmt::format("Definition of function {}() has a non-name argument {})", lhs, arg));
            }
        }

        return r;
    }

    std::string string() const noexcept override {
        auto s = fmt::format("({}(", *lhs);
        int i = 0;
        for (ttlet &arg: args) {
            if (i++ > 0) {
                s += ',';
                s += ' ';
            }
            s += to_string(*arg);
        }
        return s + "))";
    }
};

}