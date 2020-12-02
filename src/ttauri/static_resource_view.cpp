// Copyright 2019 Pokitec
// All rights reserved.

#include "static_resource_view.hpp"
#include "application.hpp"

namespace tt {

static_resource_view::static_resource_view(std::string const &filename) :
    _bytes(static_resource_view::get_static_resource(filename))
{
}

void static_resource_view::add_static_resource(std::string const &key, std::span<std::byte const> value) noexcept
{
    _static_resources.try_emplace(key, value);
}

std::span<std::byte const> static_resource_view::get_static_resource(std::string const &key)
{
    ttlet i = _static_resources.find(key);
    if (i == _static_resources.end()) {
        TTAURI_THROW(key_error("Could not find static resource").set<key_tag>(key));
    }
    return i->second;
}

}
