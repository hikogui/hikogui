
#pragma once

#include "ASTStatement.hpp"
#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTPrefix : ASTStatement {
    ASTExpression *key;

    ASTPrefix(ASTLocation location, ASTExpression *key) : ASTStatement(location), key(key) {}
    ~ASTPrefix() {
        delete key;
    }

    std::string str() override {
        return "[" + key->str() + "]";
    }
};

}
