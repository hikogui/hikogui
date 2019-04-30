// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "AST.hpp"

namespace TTauri::Config {

struct ParseContext {
    ASTObject *object;

    std::shared_ptr<std::filesystem::path> file;
    Location errorLocation;
    std::string errorMessage;

    ParseContext(const std::filesystem::path &path) :
        object(nullptr), file(std::make_shared<std::filesystem::path>(std::move(path))) {}

    void setError(Location location, std::string message) {
        this->errorLocation = std::move(location);
        this->errorMessage = std::move(message);
    }
};

}
