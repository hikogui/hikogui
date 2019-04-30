// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTNode.hpp"

namespace TTauri::Config {

struct ASTExpression : ASTNode {
    ASTExpression(Location location) : ASTNode(location) {}
};

}
