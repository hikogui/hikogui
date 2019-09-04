// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "StaticResources.hpp"
#include <gsl/gsl>
#include <cstddef>
#include <unordered_map>

namespace TTauri {


struct StaticResourceView {
    gsl::span<std::byte const> bytes;

    StaticResourceView(std::string const &filename) : bytes(get_singleton<StaticResources>().get(filename)) {}
};


}
