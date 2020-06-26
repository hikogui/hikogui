// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "observable.hpp"
#include <string>
#include <vector>

namespace tt {

inline observable<std::vector<std::string>> language_list = std::vector<std::string>{"en-US"};

[[nodiscard]] std::vector<std::string> read_os_language_list() noexcept; 


}