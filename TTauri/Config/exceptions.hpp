// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/exception/all.hpp>
#include <exception>
#include <string>

namespace TTauri::Config {

using errinfo_location = boost::error_info<struct tag_location,Location>;
using errinfo_message = boost::error_info<struct tag_message,std::string>;

struct ConfigError : virtual boost::exception, virtual std::exception {};
struct ParseError : virtual ConfigError {};
struct InvalidOperationError : virtual ConfigError {};
struct IOError : virtual ConfigError {};
struct InternalParserError : virtual ConfigError {};

}
