// Copyright Take Vos 2019, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "void_span.hpp"
#include <span>
#include <variant>
#include <functional>
#include <cstddef>
#include <string_view>
#include <type_traits>

namespace hi::inline v1 {
class URL;

/** A read-only memory mapping of a resource.
 */
class resource_view {
public:
    resource_view() = default;
    virtual ~resource_view() = default;
    resource_view(resource_view const& other) = default;
    resource_view(resource_view&& other) = default;
    resource_view& operator=(resource_view const& other) = default;
    resource_view& operator=(resource_view&& other) = default;

    /** Offset into the resource file.
     * @return offset into the resource file.
     */
    [[nodiscard]] virtual std::size_t offset() const noexcept = 0;


    /** Get a span to the memory mapping.
     */
    [[nodiscard]] virtual const_void_span span() const noexcept = 0;
};

class writable_resource_view : public resource_view {
public:
    /** Get a span to the memory mapping.
     */
    [[nodiscard]] virtual void_span writable_span() noexcept = 0;
};

/** Get a span to the memory mapping.
 */
[[nodiscard]] inline std::string_view as_string_view(resource_view const &rhs) noexcept
{
    return as_string_view(rhs.span());
}

[[nodiscard]] inline bstring_view as_bstring_view(resource_view const &rhs) noexcept
{
    return as_bstring_view(rhs.span());
}

template<typename T, size_t E = std::dynamic_extent>
[[nodiscard]] inline std::span<T, E> as_span(resource_view const& rhs) noexcept requires(std::is_const_v<T>)
{
    return as_span<T, E>(rhs.span());
}

template<typename T, size_t E = std::dynamic_extent>
[[nodiscard]] inline std::span<T, E> as_writable_span(writable_resource_view& rhs) noexcept
{
    return as_span<T, E>(rhs.writable_span());
}

} // namespace hi::inline v1
