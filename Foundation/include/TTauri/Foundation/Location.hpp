// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/URL.hpp"
#include <fmt/format.h>
#include <memory>
#include <iostream>

namespace TTauri {

/*! Location inside a configuration file.
 */
struct Location {
    /** The URL to the file that was parsed.
     * This is a shared_ptr, since a lot of Location objects will point to the same file.
     */
    std::shared_ptr<URL> file;

    /** Line where the token was found.
     * Starts at 1.
     */
    int line;

    /** Column where the token was found.
     * Starts at 1.
     */
    int column;

    /** Construct an empty location object.
     */
    Location() noexcept : file({}), line(0), column(0) {}

    /** Construct a location.
     * @param file An URL to the file where the token was found.
     * @param line Line number where the token was found.
     * @param column Column where the token was found.
     */
    Location(std::shared_ptr<URL> const &file, int line, int column) noexcept : file(file), line(line), column(column) {}
    
    friend std::string to_string(Location const &l) noexcept {
        return fmt::format("{0}:{1}:{2}", *l.file, l.line, l.column);
    }

    friend std::ostream& operator<<(std::ostream &os, Location const &l) {
        os << to_string(l);
        return os;
    }
};


}


