// Copyright 2019 Pokitec
// All rights reserved.

#include "Font.hpp"
#include "TrueTypeFont.hpp"

namespace TTauri {

template<>
std::unique_ptr<Draw::Font> parseResource(URL const &location)
{
    let view = ResourceView(location);

    if (location.extension() == "ttf") {
        try {
            return std::make_unique<Draw::TrueTypeFont>(view);
        } catch (error &e) {
            e.set<"url"_tag>(location);
            throw;
        }

    } else {
        TTAURI_THROW(url_error("Unknown extension")
            .set<"url"_tag>(location)
        );
    }
}

}