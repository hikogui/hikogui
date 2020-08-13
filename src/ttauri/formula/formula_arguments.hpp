// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "formula_node.hpp"

namespace tt {

/** A temporary node used during parsing.
*/
struct formula_arguments final : formula_node {
    formula_vector args;

    formula_arguments(parse_location location, formula_vector args) :
        formula_node(std::move(location)), args(std::move(args)) {}

    formula_arguments(parse_location location, std::unique_ptr<formula_node> arg1, std::unique_ptr<formula_node> arg2) :
        formula_node(std::move(location))
    {
        args.push_back(std::move(arg1));
        args.push_back(std::move(arg2));
    }

    datum evaluate(formula_evaluation_context& context) const override {
        return {};
    };

    std::string string() const noexcept override {
        std::string s = "<args ";
        int i = 0;
        for (ttlet &arg: args) {
            if (i++ > 0) {
                s += ", ";
            }
            s += arg->string();
        }
        return s + ">";
    }
};

}