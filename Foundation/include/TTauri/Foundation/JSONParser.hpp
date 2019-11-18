// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/tokenizer.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/datum.hpp"
#include <string>
#include <optional>
#include <string_view>

namespace TTauri {

datum parseJSON(std::string_view text);

datum parseJSON(TTauri::URL const &file);

}
