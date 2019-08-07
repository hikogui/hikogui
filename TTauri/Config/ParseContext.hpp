// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri::Config {

struct ASTObject;

/*! Context used during parsing.
 */
struct ParseContext {
    //! Object to be returned by bison after parsing.
    ASTObject *object;

    //! File being parsed.
    std::shared_ptr<URL> file;

    //! Location of an error detected by bison.
    Location errorLocation;

    //! Error message from bison.
    std::string errorMessage;

    ParseContext(const URL &path) :
        object(nullptr), file(std::make_shared<URL>(std::move(path))) {}

    /*! Set the error during parsing.
     * Called by bison during parsing on error.
     */
    void setError(Location location, std::string message) {
        this->errorLocation = std::move(location);
        this->errorMessage = std::move(message);
    }
};

}
