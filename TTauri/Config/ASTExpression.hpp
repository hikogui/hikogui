// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTNode.hpp"

namespace TTauri::Config {

struct ASTExpression : ASTNode {
    ASTExpression(Location location) : ASTNode(location) {}

    virtual std::vector<std::string> getFQName() {
        BOOST_THROW_EXCEPTION(InvalidOperationError("Expression does not represent a fully qualified name.")
            << errinfo_location(location)
        );
    }
};

}
