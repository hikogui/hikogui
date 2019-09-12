// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
#include "TTauri/StaticResources.hpp"
#include <gsl/gsl>
#include <cstddef>
#include <unordered_map>

namespace TTauri {


struct StaticResourceView {
    // Borrowed reference from a byte array inside StaticResources.
    gsl::span<std::byte const> _bytes;

    StaticResourceView(std::string const &filename) : _bytes(get_singleton<StaticResources>().get(filename)) {}

    StaticResourceView() = delete;
    ~StaticResourceView() = default;
    StaticResourceView(StaticResourceView const &other) = default;
    StaticResourceView &operator=(StaticResourceView const &other) = default;
    StaticResourceView(StaticResourceView &&other) = default;
    StaticResourceView &operator=(StaticResourceView &&other) = default;

    gsl::span<std::byte const> bytes() const noexcept { return _bytes; }
};


}
