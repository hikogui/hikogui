// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "resource_view.hpp"
#include "utility.hpp"
#include <span>
#include <cstddef>
#include <unordered_map>

namespace hi::inline v1 {

/** A resource that was included in the executable.
 */
class static_resource_view : public resource_view {
public:
    static_resource_view(std::string const &filename);

    static_resource_view() = delete;
    ~static_resource_view() = default;
    static_resource_view(static_resource_view const &other) = default;
    static_resource_view &operator=(static_resource_view const &other) = default;
    static_resource_view(static_resource_view &&other) = default;
    static_resource_view &operator=(static_resource_view &&other) = default;

    [[nodiscard]] std::size_t offset() const noexcept override
    {
        return 0;
    }

    [[nodiscard]] const_void_span span() const noexcept override
    {
        return _bytes;
    }

    [[nodiscard]] static std::unique_ptr<resource_view> loadView(std::string const &location)
    {
        return std::make_unique<static_resource_view>(location);
    }

    /** Get the data of a static resource.
     * These are resources that where linked into the executable.
     *
     * @param filename Name of the resource.
     * @return A span to the constant byte array.
     * @exception key_error Thrown when static resource could not be found.
     */
    static const_void_span get_static_resource(std::string const &filename);

private:
    // Borrowed reference from a byte array inside StaticResources.
    const_void_span _bytes;
};

} // namespace hi::inline v1
