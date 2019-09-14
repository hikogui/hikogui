// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/ASTExpression.hpp"
#include <vector>

namespace TTauri::Config {

/*! Temporary node holding a list of expressions.
 * instances only exists during the execution of bison.
 */
struct ASTExpressionList : ASTNode {
    std::vector<ASTExpression *> expressions;

    ASTExpressionList(Location location, ASTExpression *firstExpression) noexcept : ASTNode(location), expressions({firstExpression}) {}

    ~ASTExpressionList() {
        for (auto expression: expressions) {
            delete expression;
        }
    }

    ASTExpressionList() = delete;
    ASTExpressionList(ASTExpressionList const &node) = delete;
    ASTExpressionList(ASTExpressionList &&node) = delete;
    ASTExpressionList &operator=(ASTExpressionList const &node) = delete;
    ASTExpressionList &operator=(ASTExpressionList &&node) = delete;

    std::string string() const noexcept override {
        no_default;
    }

    void add(ASTExpression *x) noexcept {
        expressions.push_back(x);
    }
};

}
