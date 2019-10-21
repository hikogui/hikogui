// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/Font.hpp"
#include "TTauri/Foundation/TrueTypeFont.hpp"
#include "TTauri/Foundation/ResourceView.hpp"

namespace TTauri {

template<>
std::unique_ptr<Font> parseResource(URL const &location)
{
    if (location.extension() == "ttf") {
        auto view = ResourceView::loadView(location);

        try {
            auto font = std::make_unique<TrueTypeFont>(std::move(view));
            return font;
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
