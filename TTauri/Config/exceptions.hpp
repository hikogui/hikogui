// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/exception/all.hpp>
#include <exception>
#include <string>

using errinfo_at_column = boost::error_info<struct tag_at_column,int>;
using errinfo_message = boost::error_info<struct tag_message,std::string>;
struct InvalidOperationError : virtual boost::exception, virtual std::exception {};
struct CanNotOpenFile : virtual boost::exception, virtual std::exception {};
struct CanNotCloseFile : virtual boost::exception, virtual std::exception {};
struct InternalParserError : virtual boost::exception, virtual std::exception {};
struct ParseError : virtual boost::exception, virtual std::exception {};
