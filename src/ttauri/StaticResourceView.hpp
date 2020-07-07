// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/ResourceView.hpp"
#include "ttauri/required.hpp"
#include <nonstd/span>
#include <cstddef>
#include <unordered_map>

namespace tt {

/** A resource that was included in the executable.
 */
class StaticResourceView : public ResourceView {
private:
    // Borrowed reference from a byte array inside StaticResources.
    nonstd::span<std::byte const> _bytes;

public:
    StaticResourceView(std::string const &filename);

    StaticResourceView() = delete;
    ~StaticResourceView() = default;
    StaticResourceView(StaticResourceView const &other) = default;
    StaticResourceView &operator=(StaticResourceView const &other) = default;
    StaticResourceView(StaticResourceView &&other) = default;
    StaticResourceView &operator=(StaticResourceView &&other) = default;

    [[nodiscard]] size_t offset() const noexcept override { return 0; }

    [[nodiscard]] size_t size() const noexcept override { return _bytes.size(); }

    [[nodiscard]] std::byte const *data() const noexcept override { return _bytes.data(); }

    [[nodiscard]] nonstd::span<std::byte const> bytes() const noexcept override { return _bytes; }

    [[nodiscard]] std::string_view string_view() const noexcept override { return {reinterpret_cast<char const*>(data()), size()}; }

    [[nodiscard]] static std::unique_ptr<ResourceView> loadView(std::string const &location) {
        return std::make_unique<StaticResourceView>(location);
    }
};


}
