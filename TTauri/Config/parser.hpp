// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <filesystem>
#include <boost/exception/all.hpp>

namespace TTauri::Config {

struct ASTObject;

ASTObject *parseConfigFile(const std::filesystem::path &path);

}

