// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/URL.hpp"
#include <boost/exception/all.hpp>

namespace TTauri::Config {

struct ASTObject;

ASTObject *parseConfigFile(URL const &path);

}

