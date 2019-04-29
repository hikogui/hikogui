
#pragma once

#include "ASTNode.hpp"

namespace TTauri::Config {

struct ASTExpression : ASTNode {
    ASTExpression(ASTLocation location) : ASTNode(location) {}
};

}
