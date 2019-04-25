#pragma once

#include "AST.hpp"

namespace TTauri::Config {

struct ParseContext {
    ASTObject *object;
    ASTLocation errorLocation;
    std::string errorMessage;

    void setError(ASTLocation location, std::string message) {
        this->errorLocation = std::move(location);
        this->errorMessage = std::move(message);
    }
};

}