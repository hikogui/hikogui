// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <format>
#include <memory>
#include <iostream>
#include <string_view>

hi_export_module(hikogui.parser.parse_location);

hi_export namespace hi::inline v1 {

/*! Location inside a configuration file.
 */
class parse_location {
    /** The path to the file that was parsed.
     * This is a shared_ptr, since a lot of Location objects will point to the same file.
     */
    std::string _file = {};

    /** Line where the token was found.
     * Starts at 0.
     */
    int _line = 0;

    /** Column where the token was found.
     * Starts at 0.
     */
    int _column = 0;

public:
    /** Construct an empty location object.
     */
    constexpr parse_location() noexcept = default;

    /** Construct a location.
     * @param file A path to the file where the token was found.
     */
    constexpr parse_location(std::string file, int line = 1, int column = 1) noexcept : _file(std::move(file)), _line(line - 1), _column(column - 1) {}

    /** Construct a location.
     * @param line Line number where the token was found.
     * @param column Column where the token was found.
     */
    constexpr parse_location(int line, int column) noexcept : parse_location(std::string{}, line, column) {}

    [[nodiscard]] constexpr bool has_file() const noexcept
    {
        return not _file.empty();
    }

    [[nodiscard]] constexpr std::string file() const noexcept
    {
        return _file;
    }

    [[nodiscard]] constexpr int line() const noexcept
    {
        return _line + 1;
    }

    [[nodiscard]] constexpr int column() const noexcept
    {
        return _column + 1;
    }

    [[nodiscard]] constexpr std::pair<int, int> line_and_column() const noexcept
    {
        return {_line + 1, _column + 1};
    }

    constexpr void set_file(std::string file) noexcept
    {
        _file = std::move(file);
    }

    constexpr void set_line(int line) noexcept
    {
        _line = line - 1;
    }

    constexpr void set_column(int column) noexcept
    {
        _column = column - 1;
    }

    constexpr void set_line_and_column(std::pair<int, int> line_and_column) noexcept
    {
        _line = line_and_column.first - 1;
        _column = line_and_column.second - 1;
    }

    constexpr void increment_column() noexcept
    {
        ++_column;
    }

    constexpr void tab_column() noexcept
    {
        _column /= 8;
        _column += 1;
        _column *= 8;
    }

    constexpr void increment_line() noexcept
    {
        _column = 0;
        ++_line;
    }

    constexpr parse_location& operator+=(char c) noexcept
    {
        switch (c) {
        case '\t':
            _column = ((_column / 8) + 1) * 8;
            break;
        case '\f':
            [[fallthrough]];
        case '\n':
            ++_line;
            [[fallthrough]];
        case '\r':
            _column = 0;
            break;
        default:
            ++_column;
        }
        return *this;
    }

    constexpr parse_location& operator+=(std::string const& s) noexcept
    {
        for (hilet c : s) {
            *this += c;
        }
        return *this;
    }

    constexpr parse_location& operator+=(char const *s) noexcept
    {
        hi_assert_not_null(s);
        while (hilet c = *s++) {
            *this += c;
        }
        return *this;
    }

    constexpr parse_location& operator+=(parse_location const& location) noexcept
    {
        if (location._line == 0) {
            _column += location._column;
        } else {
            _line += location._line;
            _column = location._column;
        }
        return *this;
    }

    [[nodiscard]] friend std::string to_string(parse_location const& l) noexcept
    {
        return std::format("{}:{}:{}", l.file(), l.line(), l.column());
    }

    friend std::ostream& operator<<(std::ostream& os, parse_location const& l)
    {
        os << to_string(l);
        return os;
    }
};

} // namespace hi::inline v1

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::parse_location, char> : std::formatter<string_view, char> {
    auto format(hi::parse_location t, auto& fc) const
    {
        return std::formatter<string_view, char>::format(to_string(t), fc);
    }
};
