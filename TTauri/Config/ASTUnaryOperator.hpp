
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTUnaryOperator : ASTExpression {
    enum class Operator {
        NOT,
        NEG,
        LOGICAL_NOT
    };

    Operator op;
    ASTExpression *right;

    ASTUnaryOperator(ASTLocation location, Operator op, ASTExpression *right) : ASTExpression(location), op(op), right(right) {}

    ~ASTUnaryOperator() {
        delete right;
    }

    std::string str() override {
        std::string s;

        switch (op) {
        case Operator::NOT: s = "~"; break;
        case Operator::NEG: s = "-"; break;
        case Operator::LOGICAL_NOT: s = "not "; break;
        }

        return s + right->str();
    }

    virtual std::shared_ptr<ValueBase> execute(ExecutionContext *context) override {
        switch (op) {
        case Operator::NOT: return ~(*right->execute(context));
        case Operator::NEG: return -(*right->execute(context));
        case Operator::LOGICAL_NOT: return !(*right->execute(context));
        }
    }
};

}