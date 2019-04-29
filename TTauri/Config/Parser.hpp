// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "AST.hpp"
#include <filesystem>
#include <boost/exception/all.hpp>

namespace TTauri::Config {

    struct CanNotOpenFile : virtual boost::exception, virtual std::exception {};
    struct CanNotCloseFile : virtual boost::exception, virtual std::exception {};
    struct InternalParserError : virtual boost::exception, virtual std::exception {};

ASTObject *parseFile(const std::filesystem::path &path);

}

