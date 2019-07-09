// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "StaticResources.hpp"
#include "utils.hpp"
#include <gsl/gsl>
#include <filesystem>
#include <cstddef>
#include <unordered_map>

namespace TTauri {


struct StaticResourceView {
    gsl::span<std::byte const> bytes;

    StaticResourceView(URL const &location) : bytes(get_singleton<StaticResources>().get(location)) {}

    static int add(uint8_t const *data, size_t size, URL const &location);

    static inline std::unordered_map<URL, gsl::span<std::byte const>> objects;
};


}