// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Location.hpp"
#include "TTauri/exceptions.hpp"
#include <string>

namespace TTauri::Config {

using errinfo_location = boost::error_info<struct tag_location,Location>;
using errinfo_previous_error_message = boost::error_info<struct tag_previous_error_message,std::string>;

}
