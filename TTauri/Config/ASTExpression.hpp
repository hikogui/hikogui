// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTNode.hpp"

namespace TTauri::Config {

struct ASTExpression : ASTNode {
    ASTExpression(Location location) noexcept : ASTNode(location) {}

    virtual std::vector<std::string> getFQName() {
        TTAURI_THROW(invalid_operation_error("Expression does not represent a fully qualified name.")
            << error_info("location", location)
        );
    }
};

}
