
#pragma once

#include "ASTNode.hpp"

namespace TTauri::Config {

struct ASTStatement : ASTNode {
    ASTStatement(ASTLocation location) : ASTNode(location) {}

    std::string str() override {
        return "<statement>";
    }
};

}
