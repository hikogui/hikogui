// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <cstdint>
#include "TTauri/URL.hpp"

namespace TTauri::Config {

char *parseIdentifier(const char *text);

char *parseString(const char *text);

double parseFloat(const char *text);

int64_t parseInteger(const char *text, int radix, bool negative);

bool parseBoolean(const char *text);

URL *parseURL(const char *text);

}
