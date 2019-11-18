// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/tokenizer.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <string>
#include <optional>
#include <string_view>

namespace Messari {

ASTRoot parseJSON(std::string_view text);

ASTRoot parseJSON(TTauri::URL const &file);

}
