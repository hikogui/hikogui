// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "URL.hpp"
#include "exceptions.hpp"
#include <gsl/gsl>
#include <unordered_map>
#include <cstdint>
#include <cstddef>

namespace TTauri {

struct StaticResources {
    std::unordered_map<std::string,gsl::span<std::byte const>> intrinsic;

    StaticResources();

    gsl::span<std::byte const> const &get(std::string const &filename) const;
};

}