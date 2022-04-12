// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <span>
#include <variant>
#include <functional>
#include <cstddef>
#include <string_view>

namespace hi::inline v1 {
class URL;

/** A read-only memory mapping of a resource.
 */
class resource_view {
public:
    resource_view() = default;
    virtual ~resource_view() = default;
    resource_view(resource_view const &other) = default;
    resource_view(resource_view &&other) = default;
    resource_view &operator=(resource_view const &other) = default;
    resource_view &operator=(resource_view &&other) = default;

    /** Offset into the resource file.
     * @return offset into the resource file.
     */
    [[nodiscard]] virtual std::size_t offset() const noexcept = 0;

    /** Get a span to the memory mapping.
     */
    [[nodiscard]] virtual std::span<std::byte const> bytes() const noexcept = 0;

    /** Get a span to the memory mapping.
     */
    [[nodiscard]] virtual std::string_view string_view() const noexcept = 0;

    operator std::span<std::byte const>() const noexcept
    {
        return bytes();
    }

    /** Size of the memory mapping.
     */
    [[nodiscard]] virtual std::size_t size() const noexcept = 0;

    /** Pointer to the memory mapping.
     */
    [[nodiscard]] virtual std::byte const *data() const noexcept = 0;
};

} // namespace hi::inline v1
