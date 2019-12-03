// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/required.hpp"
#include <gsl/gsl>
#include <cstddef>
#include <unordered_map>

namespace TTauri {

/** A resource that was included in the executable.
 */
class StaticResourceView : public ResourceView {
private:
    // Borrowed reference from a byte array inside StaticResources.
    gsl::span<std::byte const> _bytes;

public:
    StaticResourceView(std::string const &filename) : _bytes(Foundation_globals->getStaticResource(filename)) {}

    StaticResourceView() = delete;
    ~StaticResourceView() = default;
    StaticResourceView(StaticResourceView const &other) = default;
    StaticResourceView &operator=(StaticResourceView const &other) = default;
    StaticResourceView(StaticResourceView &&other) = default;
    StaticResourceView &operator=(StaticResourceView &&other) = default;

    size_t offset() const noexcept override { return 0; }

    size_t size() const noexcept override { return _bytes.size(); }

    void const *data() const noexcept override { return _bytes.data(); }

    gsl::span<std::byte const> bytes() const noexcept override { return _bytes; }

    static std::unique_ptr<ResourceView> loadView(std::string const &location) {
        return std::make_unique<StaticResourceView>(location);
    }
};


}
