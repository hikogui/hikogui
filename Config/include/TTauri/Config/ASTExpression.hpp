// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Config/ASTNode.hpp"

namespace TTauri::Config {

struct ASTExpression : ASTNode {
    ASTExpression(Location location) noexcept : ASTNode(location) {}

    virtual std::vector<std::string> getFQName() {
        TTAURI_THROW(invalid_operation_error("Expression does not represent a fully qualified name.")
            .set<"url"_tag>(*location.file).set<"line"_tag>(location.line).set<"column"_tag>(location.column)
        );
    }
};

}
