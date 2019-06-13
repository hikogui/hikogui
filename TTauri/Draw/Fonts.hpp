// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Font.hpp"
#include "TrueTypeParser.hpp"
#include <filesystem>
#include <map>

namespace TTauri::Draw {

struct Fonts {
    std::map<std::filesystem::path, Font> fonts;

    Font const &get(std::filesystem::path path) {
        let i = fonts.find(path);
        if (i == fonts.end()) {
            return fonts[path] = parseTrueTypeFile(path);
        } else {
            return i->second;
        }
    }
};

inline std::unique_ptr<Fonts> fonts;

}
