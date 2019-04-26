
#pragma once

#include "ASTExpression.hpp"

#include <vector>

namespace TTauri::Config {

struct ASTExpressions : ASTNode {
    std::vector<ASTExpression *> expressions;

    ASTExpressions(ASTLocation location, ASTExpression *firstExpression) : ASTNode(location), expressions({firstExpression}) {}

    ~ASTExpressions() {
        for (auto expression: expressions) {
            delete expression;
        }
    }

    std::string str() override {
        return "<expressions>";
    }
};

}
