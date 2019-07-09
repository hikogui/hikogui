// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Draw/Font.hpp"
#include "URL.hpp"
#include <gsl/gsl>
#include <cstddef>
#include <variant>
#include <unordered_map>

namespace TTauri {

using Resource = std::variant<Draw::Font>;

Resource parseResource(URL const &location);

struct Resources {
    std::unordered_map<URL,Resource> resourceCache;

    /*! Return parsed resource.
    */
    template <typename T>
    T &get(URL const &location) {
        let i = resourceCache.find(location);
        if (i != resourceCache.end()) {
            return std::get<T>(i->second);
        }

        let [newResource, wasAdded] = resourceCache.try_emplace(location, parseResource(location));

        return std::get<T>(newResource->second);
    }
};

}