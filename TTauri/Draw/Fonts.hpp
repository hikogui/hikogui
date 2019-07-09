// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Font.hpp"
#include "TrueTypeParser.hpp"
#include "TTauri/ResourceView.hpp"
#include <filesystem>
#include <map>

namespace TTauri::Draw {

struct Fonts {
    std::map<URL, Font> fonts;

    Font const &get(URL const &location) {
        let i = fonts.find(location);
        if (i == fonts.end()) {
            let view = ResourceView{location};
            return fonts[location] = parseTrueTypeFile(view.bytes());
        } else {
            return i->second;
        }
    }
};

inline std::unique_ptr<Fonts> fonts;

}
