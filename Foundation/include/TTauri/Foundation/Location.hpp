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
    std::shared_ptr<URL> file;
    int line;
    int column;

    Location() noexcept : file({}), line(0), column(0) {}

    Location(std::shared_ptr<URL> const &file, int line, int column) noexcept : file(file), line(line), column(column) {}
    
    std::string string() const noexcept {
        return fmt::format("{0}:{1}:{2}", *file, line, column);
    }
};

inline std::ostream& operator<<(std::ostream &os, const Location &l)
{
    os << l.string();
    return os;
}

}


