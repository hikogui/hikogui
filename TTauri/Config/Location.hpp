// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/URL.hpp"
#include <memory>
#include <iostream>
#include <boost/format.hpp>

namespace TTauri::Config {

/*! Location inside a configuration file.
 */
struct Location {
    std::shared_ptr<URL> file;
    int line;
    int column;

    Location() noexcept : file({}), line(0), column(0) {}

    Location(std::shared_ptr<URL> const &file, int line, int column) noexcept : file(file), line(line), column(column) {}

    
    std::string string() const noexcept {
        return (boost::format("%s:%i:%i") % (*file) % line % column).str();
    }
};

inline std::ostream& operator<<(std::ostream &os, const Location &l)
{
    os << l.string();
    return os;
}

}


