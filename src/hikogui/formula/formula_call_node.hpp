// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_node.hpp"

namespace hi::inline v1 {

struct formula_call_node final : formula_node {
    std::unique_ptr<formula_node> lhs;
    formula_vector args;

    formula_call_node(parse_location location, std::unique_ptr<formula_node> lhs, formula_node &rhs) :
        formula_node(std::move(location)), lhs(std::move(lhs))
    {
        auto &rhs_ = dynamic_cast<formula_arguments &>(rhs);
        args = std::move(rhs_.args);
    }

    void post_process(formula_post_process_context &context) override
    {
        lhs->resolve_function_pointer(context);
        for (auto &arg : args) {
            arg->post_process(context);
        }
    }

    datum evaluate(formula_evaluation_context &context) const override
    {
        hilet args_ = transform<datum::vector_type>(args, [&](hilet &x) {
            return x->evaluate(context);
        });

        return lhs->call(context, args_);
    }

    std::vector<std::string> get_name_and_argument_names() const override
    {
        std::vector<std::string> r;

        try {
            r.push_back(lhs->get_name());
        } catch (parse_error const &e) {
            throw parse_error(std::format("Function definition does not have a name, got {}\n{}", *lhs, e.what()));
        }

        for (hilet &arg : args) {
            try {
                r.push_back(arg->get_name());
            } catch (parse_error const &e) {
                throw parse_error(std::format("Definition of function {}() has a non-name argument {}\n{}", *lhs, *arg, e.what()));
            }
        }

        return r;
    }

    std::string string() const noexcept override
    {
        auto s = std::format("({}(", *lhs);
        int i = 0;
        for (hilet &arg : args) {
            if (i++ > 0) {
                s += ',';
                s += ' ';
            }
            s += to_string(*arg);
        }
        return s + "))";
    }
};

} // namespace hi::inline v1