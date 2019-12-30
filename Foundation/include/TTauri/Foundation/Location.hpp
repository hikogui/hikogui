// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include <fmt/format.h>
#include <memory>
#include <iostream>

namespace TTauri {

/*! Location inside a configuration file.
 */
class Location {
    /** The URL to the file that was parsed.
     * This is a shared_ptr, since a lot of Location objects will point to the same file.
     */
    std::shared_ptr<URL> _file;

    /** Line where the token was found.
     * Starts at 0.
     */
    int _line;

    /** Column where the token was found.
     * Starts at 0.
     */
    int _column;

public:
    /** Construct an empty location object.
     */
    Location() noexcept : _file({}), _line(0), _column(0) {}

    /** Construct a location.
     * @param file An URL to the file where the token was found.
     * @param line Line number where the token was found.
     * @param column Column where the token was found.
     */
    Location(std::shared_ptr<URL> const &file, int line, int column) noexcept : _file(file), _line(line - 1), _column(column - 1) {}
    
    [[nodiscard]] bool has_file() const noexcept {
        return static_cast<bool>(_file);
    }

    [[nodiscard]] URL const &file() const noexcept {
        return *_file;
    }

    [[nodiscard]] int line() const noexcept {
        return _line + 1;
    }

    [[nodiscard]] int column() const noexcept {
        return _column + 1;
    }

    [[nodiscard]] std::pair<int,int> line_and_column() const noexcept {
        return {_line + 1, _column + 1};
    }

    void set_file(std::shared_ptr<URL> file) {
        _file = std::move(file);
    }

    void set_line(int line) noexcept {
        _line = line - 1;
    }

    void set_column(int column) noexcept {
        _column = column - 1;
    }

    void set_line_and_column(std::pair<int,int> line_and_column) noexcept {
        _line = line_and_column.first - 1;
        _column = line_and_column.second - 1;
    }

    Location &operator+=(char c) noexcept {
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

    Location &operator+=(std::string const &s) noexcept {
        for (let c: s) {
            *this += c;
        }
        return *this;
    }

    Location &operator+=(char const *s) noexcept {
        while (let c = *s++) {
            *this += c;
        }
        return *this;
    }

    Location &operator+=(Location const &location) noexcept {
        if (location._line == 0) {
            _column += location._column;
        } else {
            _line += location._line;
            _column = location._column;
        }
        return *this;
    }

    friend std::string to_string(Location const &l) noexcept {
        return fmt::format("{0}:{1}:{2}", l.file(), l.line(), l.column());
    }

    friend std::ostream& operator<<(std::ostream &os, Location const &l) {
        os << to_string(l);
        return os;
    }
};


}


