
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTString : ASTExpression {
    std::string value;

    ASTString(ASTLocation location, char *value) : ASTExpression(location), value(value) {
        free(value);
    }

    std::string str() override {
        return "\"" + value + "\"";
    }
};

}
