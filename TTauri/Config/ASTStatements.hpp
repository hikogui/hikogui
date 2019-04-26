
#pragma once

#include "ASTNode.hpp"
#include "ASTStatement.hpp"
#include <vector>

namespace TTauri::Config {

struct ASTStatements : ASTNode {
    std::vector<ASTStatement *> statements;

    ASTStatements(ASTLocation location, ASTStatement *firstStatement) : ASTNode(location), statements({firstStatement}) {}

    ~ASTStatements() {
        for (auto statement: statements) {
            delete statement;
        }
    }

    std::string str() override {
        return "<statements>";
    }
};

}
