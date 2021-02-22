// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "formula_node.hpp"

namespace tt {

struct formula_vector_literal_node final : formula_node {
    formula_vector values;

    formula_vector_literal_node(parse_location location, formula_vector values) :
        formula_node(std::move(location)), values(std::move(values)) {}

    void post_process(formula_post_process_context& context) override {
        for (auto &value: values) {
            value->post_process(context);
        }
    }

    datum evaluate(formula_evaluation_context& context) const override {
        datum::vector r;
        for (ttlet &value: values) {
            r.push_back(value->evaluate(context));
        }
        return datum{std::move(r)};
    }

    datum &assign(formula_evaluation_context& context, datum const &rhs) const override {
        if (!rhs.is_vector()) {
            throw operation_error("{}: Unpacking values can only be done on vectors, got {}.", location, rhs);
        }
        if (values.size() < 1) {
            throw operation_error("{}: Unpacking can only be done on 1 or more return values.", location);
        }
        if (values.size() != rhs.size()) {
            throw operation_error("{}: Unpacking values can only be done on with a vector of size {} got {}.", location, values.size(), rhs.size());
        }

        // Make a copy, in case of self assignment.
        ttlet rhs_copy = rhs;

        size_t i = 0;
        while (true) {
            ttlet &lhs_ = values[i];
            ttlet &rhs_ = rhs_copy[i];

            if (++i < rhs.size()) {
                lhs_->assign(context, rhs_);
            } else {
                return lhs_->assign(context, rhs_);
            }
        }
    }

    std::string string() const noexcept override {
        std::string r = "[";
        int i = 0;
        for (ttlet &value: values) {
            if (i++ > 0) {
                r += ", ";
            }
            r += value->string();
        }
        return r + "]";
    }
};

}