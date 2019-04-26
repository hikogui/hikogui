
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTName : ASTExpression {
    std::string name;

    ASTName(ASTLocation location, char *name) : ASTExpression(location), name(name) {
        free(name);
    }

    std::string str() override {
        return name;
    }
};

}
