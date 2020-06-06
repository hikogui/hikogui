// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <nonstd/span>
#include <variant>
#include <functional>
#include <cstddef>
#include <string_view>

namespace TTauri {
class URL;

/** A read-only memory mapping of a resource.
 */
class ResourceView {
public:
    ResourceView() = default;
    virtual ~ResourceView() = default;
    ResourceView(ResourceView const &other) = default;
    ResourceView(ResourceView &&other) = default;
    ResourceView &operator=(ResourceView const &other) = default;
    ResourceView &operator=(ResourceView &&other) = default;

    /** Offset into the resource file.
     * @return offset into the resource file.
     */
    [[nodiscard]] virtual size_t offset() const noexcept = 0;

    /** Get a spam to the memory mapping.
     */
    [[nodiscard]] virtual nonstd::span<std::byte const> bytes() const noexcept = 0;

    /** Get a spam to the memory mapping.
    */
    [[nodiscard]] virtual std::string_view string_view() const noexcept = 0;

    /** Size of the memory mapping.
     */
    [[nodiscard]] virtual size_t size() const noexcept = 0;

    /** Pointer to the memory mapping.
     */
    [[nodiscard]] virtual std::byte const *data() const noexcept = 0;

    /** Load a resource.
     * @param location A `resource:` URL which will be loaded from the executable or from a file.
     * @return A pointer to a resource view.
     */
    [[nodiscard]] static std::unique_ptr<ResourceView> loadView(URL const &location);
};

}
