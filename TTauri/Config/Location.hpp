// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <filesystem>
#include <memory>

namespace TTauri::Config {

struct Location {
    std::shared_ptr<std::filesystem::path> file;
    int line;
    int column;

    Location() : file({}), line(0), column(0) {}

    Location(const std::shared_ptr<std::filesystem::path> &file, int line, int column) : file(file), line(line), column(column) {}

};

}
