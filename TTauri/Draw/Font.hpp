// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Path.hpp"
#include "PathString.hpp"
#include "TrueTypeParser.hpp"
#include "TTauri/grapheme.hpp"
#include "TTauri/required.hpp"
#include "TTauri/URL.hpp"
#include "TTauri/ResourceView.hpp"
#include <vector>
#include <map>
#include <gsl/gsl>

namespace TTauri::Draw {

struct Font {
    std::map<char32_t,size_t> characterMap;
    std::vector<Path> glyphs;

    PathString getGlyphs(gstring const &graphemes) const {
        PathString r;

        for (int graphemeIndex = 0; graphemeIndex < graphemes.size(); graphemeIndex++) {
            let &grapheme = graphemes.at(graphemeIndex);

            // XXX Try and find ligatures in font.

            // First try composed normalization
            std::vector<Path> graphemeGlyphs;
            for (let codePoint: grapheme.NFC()) {
                let i = characterMap.find(codePoint);
                if (i == characterMap.end()) {
                    graphemeGlyphs.clear();
                    break;
                }

                let glyph = glyphs.at(i->second);

                graphemeGlyphs.push_back(std::move(glyph));
            }

            if (graphemeGlyphs.size() == 0) {
                // Try again with decomposed normalization.
                for (let codePoint: grapheme.NFD()) {
                    let i = characterMap.find(codePoint);
                    if (i == characterMap.end()) {
                        graphemeGlyphs.clear();
                        break;
                    }

                    let glyph = glyphs.at(i->second);

                    graphemeGlyphs.push_back(std::move(glyph));
                }
            }

            // XXX Try fallback fonts.

            if (graphemeGlyphs.size() == 0) {
                // Replace with not-found-glyph at index 0.
                graphemeGlyphs.push_back(glyphs.at(0));
            }

            // Add graphemeGlyphs.
            for (let glyph: graphemeGlyphs) {
                r.add(std::move(glyph));
            }
        }

        return r;
    }

    PathString getGlyphs(std::string const &s) const {
        return getGlyphs(translateString<gstring>(s));
    }
};

}

namespace TTauri {

template<>
inline Draw::Font parseResource(URL const &location)
{
    let view = ResourceView(location);

    if (location.extension() == "ttf") {
        try {
            return Draw::parseTrueTypeFile(view.bytes());
        } catch (boost::exception &e) {
            e << errinfo_url(location);
            throw;
        }

    } else {
        BOOST_THROW_EXCEPTION(FileError("Unknown extension")
            << errinfo_url(location)
        );
    }
}

}
