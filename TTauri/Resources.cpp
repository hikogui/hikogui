// Copyright 2019 Pokitec
// All rights reserved.

#include "Resources.hpp"
#include "ResourceView.hpp"
#include "Draw/TrueTypeParser.hpp"

namespace TTauri {

Resource parseResource(URL const &location)
{
    let view = ResourceView(location);

    if (location.extension() == "ttf") {
        try {
            return TTauri::Draw::parseTrueTypeFile(view.bytes());
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