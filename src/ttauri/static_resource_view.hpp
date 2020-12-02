// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ResourceView.hpp"
#include "required.hpp"
#include <span>
#include <cstddef>
#include <unordered_map>

namespace tt {

/** A resource that was included in the executable.
 */
class static_resource_view : public ResourceView {
public:
    static_resource_view(std::string const &filename);

    static_resource_view() = delete;
    ~static_resource_view() = default;
    static_resource_view(static_resource_view const &other) = default;
    static_resource_view &operator=(static_resource_view const &other) = default;
    static_resource_view(static_resource_view &&other) = default;
    static_resource_view &operator=(static_resource_view &&other) = default;

    [[nodiscard]] size_t offset() const noexcept override { return 0; }

    [[nodiscard]] size_t size() const noexcept override { return _bytes.size(); }

    [[nodiscard]] std::byte const *data() const noexcept override { return _bytes.data(); }

    [[nodiscard]] std::span<std::byte const> bytes() const noexcept override { return _bytes; }

    [[nodiscard]] std::string_view string_view() const noexcept override { return {reinterpret_cast<char const*>(data()), size()}; }

    [[nodiscard]] static std::unique_ptr<ResourceView> loadView(std::string const &location) {
        return std::make_unique<static_resource_view>(location);
    }

    /** Get the data of a static resource.
     * These are resources that where linked into the exectuable.
     *
     * @param key Name of the resource.
     * @return A span to the constant byte array.
     * @exception key_error Thrown when static resource could not be found.
     */
    static std::span<std::byte const> get_static_resource(std::string const &key);

    /** Add static resource.
     * This function should only be called on resources that are linked into the executable
     * and therefor only be called by the application class.
     *
     * @param key Name of the resource.
     * @param value A span to the constant byte array
     */
    static void add_static_resource(std::string const &key, std::span<std::byte const> value) noexcept;

private:
    static inline std::unordered_map<std::string, std::span<std::byte const>> _static_resources;

    // Borrowed reference from a byte array inside StaticResources.
    std::span<std::byte const> _bytes;
};


}
