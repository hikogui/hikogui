// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ASTObject.hpp"
#include <filesystem>
#include <boost/exception/all.hpp>

namespace TTauri::Config {


ASTObject *parseConfigFile(const std::filesystem::path &path);

}

