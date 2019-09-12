// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <gsl/gsl>
#include <variant>
#include <functional>

namespace TTauri {
class URL;

class ResourceView {
public:
    ResourceView() = default;
    ~ResourceView() = default;
    ResourceView(ResourceView const &other) = default;
    ResourceView(ResourceView &&other) = default;
    ResourceView &operator=(ResourceView const &other) = default;
    ResourceView &operator=(ResourceView &&other) = default;

    virtual size_t offset() const noexcept = 0;

    virtual gsl::span<std::byte const> bytes() const noexcept = 0;

    virtual size_t size() const noexcept = 0;

    virtual void const *data() const noexcept = 0;

    static std::unique_ptr<ResourceView> loadView(URL const &location);
};

}
