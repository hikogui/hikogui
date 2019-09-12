// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Diagnostic/exceptions.hpp"
#include <gsl/gsl>
#include <string>
#include <unordered_map>
#include <cstddef>

namespace TTauri {

inline std::unordered_map<std::string,gsl::span<std::byte const>> staticResources;

inline gsl::span<std::byte const> getStaticResource(std::string const &key)
{
    let i = staticResources.find(key);
    if (i == staticResources.end()) {
        TTAURI_THROW(key_error("Could not find static resource")
            .set<"key"_tag>(key)
        );
    }
    return i->second;
}

};