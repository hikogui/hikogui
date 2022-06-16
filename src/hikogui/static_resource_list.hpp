// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "assert.hpp"
#include "void_span.hpp"
#include <span>
#include <cstddef>
#include <atomic>
#include <optional>

namespace hi::inline v1 {
struct static_resource_item;

/** The start of the list of static resource items.
 * The std::atomic pointer constructor is constexpr therefor the variable is initialized at compile time.
 * This is before any of the dynamic initializer is called, which will modify this pointer at run-time.
 */
inline std::atomic<static_resource_item const *> static_resource_list = nullptr;

struct static_resource_item {
    static_resource_item const *next;
    char const *filename;
    const_void_span bytes;

    /** Search for a static resource item.
     * @param filename The filename of the resource to search for.
     * @return A byte-span, or an empty byte-span when not found.
     */
    [[nodiscard]] static const_void_span find(std::string_view filename) noexcept
    {
        for (auto ptr = static_resource_list.load(); ptr != nullptr; ptr = ptr->next) {
            if (filename == ptr->filename) {
                return ptr->bytes;
            }
        }
        return {};
    }

    /** Add a resource item to the list.
     * This function should be used to initialize a static global variable which
     * will modify the static_resource_list.
     *
     * Example:
     * ```
     * static static_resource_item tmp1 = {nullptr, "foo", foo_bytes};
     * static static_resource_item const *tmp2 = static_resource_item::add(&tmp1);
     * ```
     * @param new_item A pointer to the new item to be added to the list.
     * @return A pointer to the previous item, should be stored into a static global variable.
     */
    [[nodiscard]] hi_no_inline static static_resource_item const *add(static_resource_item *new_item) noexcept
    {
        hi_axiom(new_item != nullptr);
        return new_item->next = static_resource_list.exchange(new_item);
    }
};

} // namespace hi::inline v1
