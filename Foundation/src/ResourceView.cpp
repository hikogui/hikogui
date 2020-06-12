// Copyright 2019 Pokitec
// All rights reserved.


#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/StaticResourceView.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/exceptions.hpp"

namespace tt {

std::unique_ptr<ResourceView> ResourceView::loadView(URL const &location)
{
    if (location.scheme() == "resource") {
        try {
            auto view = StaticResourceView::loadView(location.filename());
            LOG_INFO("Loaded resource {} from executable.", location);
            return view;

        } catch (key_error) {
            ttlet absoluteLocation = URL::urlFromResourceDirectory() / location;
            auto view = FileView::loadView(absoluteLocation);
            LOG_INFO("Loaded resource {} from filesystem at {}.", location, absoluteLocation);
            return view;
        }

    } else if (location.scheme() == "file") {
        auto view = FileView::loadView(location);
        LOG_INFO("Loaded resource {} from filesystem.", location);
        return view;

    } else {
        TTAURI_THROW(url_error("Unknown scheme for loading a resource")
            .set<url_tag>(location)
        );
    }
}

}