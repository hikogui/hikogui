
#pragma once

#include "ASTExpression.hpp"

namespace TTauri::Config {

struct ASTNull : ASTExpression {

    ASTNull(ASTLocation location) : ASTExpression(location) {}

    std::string str() override {
        return "null";
    }
};

}
