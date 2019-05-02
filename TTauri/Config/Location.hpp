// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <filesystem>
#include <memory>
#include <iostream>
#include <boost/format.hpp>

namespace TTauri::Config {

struct Location {
    std::shared_ptr<std::filesystem::path> file;
    int line;
    int column;

    Location() : file({}), line(0), column(0) {}

    Location(std::shared_ptr<std::filesystem::path> const &file, int line, int column) : file(file), line(line), column(column) {}

    std::string str() const {
        return (boost::format("%s:%i:%i") % file->string() % line % column).str();
    }
};

inline std::ostream& operator<<(std::ostream &os, const Location &l)
{
    os << l.str();
    return os;
}

}


