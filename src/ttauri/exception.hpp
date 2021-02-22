// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "fmt/format.h"
#include <exception>
#include <string_view>

#pragma once

namespace tt {

/** Exception thrown during parsing on an error.
 * This exception is often thrown due to an error in the syntax
 * in both text and binary files.
 * 
 * The what-string should start with the location of the error in the file followed with ": " and the error message.
 * The what-string may be shown to the user, when the parser was working on user supplied files.
 * 
 * The location for a text file will be: a path followed by line_nr (starting at line 1) and column_nr (starting at column 1).
 * The location for a binary: a path followed by an optional chunk names, followed by a byte number within the chunk.
 * 
 * If there are nested errors, such as an error in an included file, then the what-string may be multiple-lines, where the
 * nested error appears later in the what-string.
 */
class parse_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

    template<typename FirstArg, typename... Args>
    parse_error(std::string_view fmt, FirstArg const &arg1, Args const &... args) noexcept :
        parse_error(fmt::format(fmt, arg1, args...)) {}
};

/** Exception thrown during execution of a dynamic operation.
 * This exception is often thrown on operation between multiple polymorphic objects
 * which do not support the combined operation.
 *
 * For example a datum object may contain floating point number for which
 * a shift-right or shift-left would be an invalid operation.
 */
class operation_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

    template<typename FirstArg, typename... Args>
    operation_error(std::string_view fmt, FirstArg const &arg1, Args const &... args) noexcept :
        operation_error(fmt::format(fmt, arg1, args...))
    {
    }
};

/** Exception thrown during I/O on an error.
 * This exception is often thrown due to an error with permission or existence of files.
 *
 * The what-string should start with the path of the object where the error happened.
 * Followed after ": " with a user-friendly error message. Optionally followed between single quotes
 * the operating system error string.
 */
class io_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

    template<typename FirstArg, typename... Args>
    io_error(std::string_view fmt, FirstArg const &arg1, Args const &... args) noexcept :
        io_error(fmt::format(fmt, arg1, args...))
    {
    }
};

class gui_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

    template<typename FirstArg, typename... Args>
    gui_error(std::string_view fmt, FirstArg const &arg1, Args const &... args) noexcept :
        gui_error(fmt::format(fmt, arg1, args...))
    {
    }
};

class key_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

    template<typename FirstArg, typename... Args>
    key_error(std::string_view fmt, FirstArg const &arg1, Args const &... args) noexcept :
        key_error(fmt::format(fmt, arg1, args...))
    {
    }
};

class url_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

    template<typename FirstArg, typename... Args>
    url_error(std::string_view fmt, FirstArg const &arg1, Args const &... args) noexcept :
        url_error(fmt::format(fmt, arg1, args...))
    {
    }
};

}

