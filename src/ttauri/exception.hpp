
#include "fmt/format.h"
#include <exception>
#include <string_view>

#pragma once

namespace tt {

/** Exception thrown during parsing on an error.
 * This exception is often thrown due to an error in the syntax
 * in both text and binary files.
 *
 * The `error_info`-`parse_location_tag` is often filled in for the
 * specific file name and location inside the file where the error
 * was.
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

