
#pragma once

#include "ASTExpression.hpp"
#include "ASTStatement.hpp"
#include "ASTStatements.hpp"
#include <vector>

namespace TTauri::Config {

struct ASTObject : ASTExpression {
    std::vector<ASTStatement *> statements;

    ASTObject(ASTLocation location) : ASTExpression(location), statements() { }

    ASTObject(ASTLocation location, ASTStatement *statement) : ASTExpression(location), statements({statement}) {
    }

    ASTObject(ASTLocation location, ASTStatements *statements) : ASTExpression(location), statements(statements->statements) {
        // We copied the pointers of the expression, so they must not be destructed when expressions is deleted.
        statements->statements.clear();
        delete statements;
    }

    ~ASTObject() {
        for (auto statement: statements) {
            delete statement;
        }
    }

    std::string str() override {
        std::string s = "{";

        bool first = true;
        for (auto statement: statements) {
            if (!first) {
                s += ",";
            }
            s += statement->str();
            first = false;
        }

        s += "}";
        return s;
    }
};

}
