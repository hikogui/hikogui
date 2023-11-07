// Copyright Take Vos 2019, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file file/resource_view.hpp Defines resource_view.
 * @ingroup file
 */

#pragma once

#include "file_view.hpp"
#include "../container/container.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <span>
#include <variant>
#include <functional>
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <filesystem>
#include <memory>

hi_export_module(hikogui.file.resource_view);

hi_export namespace hi { inline namespace v1 {

namespace detail {

class resource_view_base {
public:
    virtual ~resource_view_base() = default;

    [[nodiscard]] virtual hi::const_void_span const_void_span() const noexcept = 0;
};

template<typename T>
class resource_view_impl final : public resource_view_base {
public:
    using value_type = T;

    resource_view_impl(T&& other) noexcept : _value(std::move(other)) {}
    resource_view_impl(T const& other) noexcept : _value(other) {}

    [[nodiscard]] hi::const_void_span const_void_span() const noexcept override
    {
        return _value.const_void_span();
    }

private:
    value_type _value;
};

} // namespace detail

/** A read-only view of a resource.
 *
 * This is a type erased object which holds a view to a resource
 * and exposes a common interface to access the bytes using
 * `as_span()`, `as_string_view()` or `as_bstring_view()`.
 *
 * @ingroup file
 */
class const_resource_view {
public:
    const_resource_view() = default;
    virtual ~const_resource_view() = default;
    const_resource_view(const_resource_view const& other) = default;
    const_resource_view(const_resource_view&& other) = default;
    const_resource_view& operator=(const_resource_view const& other) = default;
    const_resource_view& operator=(const_resource_view&& other) = default;

    template<typename T>
    const_resource_view(T&& view) noexcept requires requires
    {
        std::declval<std::decay_t<T>>().const_void_span();
    } : _pimpl(std::make_shared<detail::resource_view_impl<std::decay_t<T>>>(std::forward<T>(view))) {}

    const_resource_view(std::filesystem::path const& path) : const_resource_view(file_view{path}) {}

    [[nodiscard]] bool empty() const noexcept
    {
        return _pimpl == nullptr;
    }

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Get a span to the memory mapping.
     */
    [[nodiscard]] hi::const_void_span const_void_span() const noexcept
    {
        hi_assert_not_null(_pimpl);
        return _pimpl->const_void_span();
    }

    template<typename T>
    [[nodiscard]] friend std::span<T> as_span(const_resource_view const& view) noexcept
    {
        static_assert(std::is_const_v<T>);
        return as_span<T>(view.const_void_span());
    }

    [[nodiscard]] friend std::string_view as_string_view(const_resource_view const& view) noexcept
    {
        return as_string_view(view.const_void_span());
    }

    [[nodiscard]] friend bstring_view as_bstring_view(const_resource_view const& view) noexcept
    {
        return as_bstring_view(view.const_void_span());
    }

private:
    std::shared_ptr<detail::resource_view_base> _pimpl;
};

}} // namespace hi::v1
