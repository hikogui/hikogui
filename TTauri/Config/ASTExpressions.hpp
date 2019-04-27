
#pragma once

#include "ASTExpression.hpp"
#include "TTauri/utils.hpp"

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
        BOOST_THROW_EXCEPTION(NotImplementedError());
    }

    virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) override { 
        BOOST_THROW_EXCEPTION(NotImplementedError());
    } 
};

}
