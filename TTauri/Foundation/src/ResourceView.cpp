// Copyright 2019 Pokitec
// All rights reserved.


#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/StaticResourceView.hpp"

namespace TTauri {

std::unique_ptr<ResourceView> ResourceView::loadView(URL const &location)
{
    if (location.scheme() == "resource") {
        try {
            let view = StaticResourceView::loadView(location.filename());
            LOG_INFO("Loaded resource {} from executable.", location);
            return view;

        } catch (key_error) {
            let absoluteLocation = URL::urlFromResourceDirectory() / location;
            auto view = FileView::loadView(absoluteLocation);
            LOG_INFO("Loaded resource {} from filesystem at {}.", location, absoluteLocation);
            return view;
        }

    } else if (location.scheme() == "file") {
        if (!location.isAbsolute()) {
            TTAURI_THROW(url_error("file-URLs must be absolute.")
                .set<"url"_tag>(location)
            );
        }

        auto view = FileView::loadView(location);
        LOG_INFO("Loaded resource {} from filesystem.", location);
        return view;

    } else {
        TTAURI_THROW(url_error("Unknown scheme for loading a resource")
            .set<"url"_tag>(location)
        );
    }
}

}