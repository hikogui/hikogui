// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "AST.hpp"

namespace TTauri::Config {

/*! Context used during parsing.
 */
struct ParseContext {
    //! Object to be returned by bison after parsing.
    ASTObject *object;

    //! File being parsed.
    std::shared_ptr<std::filesystem::path> file;

    //! Location of an error detected by bison.
    Location errorLocation;

    //! Error message from bison.
    std::string errorMessage;

    ParseContext(const std::filesystem::path &path) :
        object(nullptr), file(std::make_shared<std::filesystem::path>(std::move(path))) {}

    /*! Set the error during parsing.
     * Called by bison during parsing on error.
     */
    void setError(Location location, std::string message) {
        this->errorLocation = std::move(location);
        this->errorMessage = std::move(message);
    }
};

}
