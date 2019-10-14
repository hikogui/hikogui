// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include <cstdint>

namespace TTauri::Config {

char *parseIdentifier(const char *text) noexcept;

char *parseString(const char *text) noexcept;

double parseFloat(const char *text) noexcept;

int64_t parseInteger(const char *text, int radix, bool negative) noexcept;

bool parseBoolean(const char *text) noexcept;

URL *parseURL(const char *text);

}
