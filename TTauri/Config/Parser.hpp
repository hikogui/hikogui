// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "AST.hpp"
#include <filesystem>
#include <boost/exception/all.hpp>

namespace TTauri::Config {


ASTObject *parseFile(const std::filesystem::path &path);

}

