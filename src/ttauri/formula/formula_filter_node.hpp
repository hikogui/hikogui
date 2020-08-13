// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "formula_binary_operator_node.hpp"
#include "formula_post_process_context.hpp"
#include "formula_name_node.hpp"

namespace tt {

struct formula_filter_node final : formula_binary_operator_node {
    mutable formula_post_process_context::filter_type filter;
    formula_name_node* rhs_name;

    formula_filter_node(parse_location location, std::unique_ptr<formula_node> lhs, std::unique_ptr<formula_node> rhs) :
        formula_binary_operator_node(std::move(location), std::move(lhs), std::move(rhs))
    {
        rhs_name = dynamic_cast<formula_name_node*>(this->rhs.get());
        if (rhs_name == nullptr) {
            TTAURI_THROW(parse_error("Expecting a name token on the right hand side of a filter operator. got {}.", rhs).set_location(location));
        }
    }

    void post_process(formula_post_process_context& context) override {
        formula_binary_operator_node::post_process(context);

        filter = context.get_filter(rhs_name->name);
        if (!filter) {
            TTAURI_THROW(parse_error("Could not find filter .{}().", rhs_name->name).set_location(location));
        }
    }

    datum evaluate(formula_evaluation_context& context) const override {
        auto lhs_ = lhs->evaluate(context);
        try {
            return {filter(static_cast<std::string>(lhs_))};
        } catch (error &e) {
            e.set_location(location);
            throw;
        }
    }

    std::string string() const noexcept override {
        return fmt::format("({} ! {})", *lhs, *rhs);
    }
};

}
