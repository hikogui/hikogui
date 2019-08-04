// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>

namespace TTauri::Config {

struct ASTObject;

ASTObject *parseConfigFile(const boost::filesystem::path &path);

}

