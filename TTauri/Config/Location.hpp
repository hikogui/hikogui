// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <memory>
#include <iostream>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

namespace TTauri::Config {

/*! Location inside a configuration file.
 */
struct Location {
    std::shared_ptr<boost::filesystem::path> file;
    int line;
    int column;

    Location() : file({}), line(0), column(0) {}

    Location(std::shared_ptr<boost::filesystem::path> const &file, int line, int column) : file(file), line(line), column(column) {}

    std::string string() const {
        return (boost::format("%s:%i:%i") % file->generic_string() % line % column).str();
    }
};

inline std::ostream& operator<<(std::ostream &os, const Location &l)
{
    os << l.string();
    return os;
}

}


