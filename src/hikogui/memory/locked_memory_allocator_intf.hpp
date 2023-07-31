// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include <type_traits>
#include <cstddef>

hi_export_module(hikogui.memory.locked_memory_allocator : intf);

hi_export namespace hi::inline v1 {

[[nodiscard]] std::byte *locked_memory_allocator_allocate(std::size_t n) noexcept;

void locked_memory_allocator_deallocate(std::byte *p, std::size_t n) noexcept;

template<typename T>
class locked_memory_allocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;

    template<typename U>
    struct rebind {
        using other = locked_memory_allocator<U>;
    };

    constexpr locked_memory_allocator() noexcept {};

    constexpr locked_memory_allocator(locked_memory_allocator const &other) noexcept {}

    template<typename U>
    constexpr locked_memory_allocator(locked_memory_allocator<U> const &other) noexcept
    {
    }

    [[nodiscard]] value_type *allocate(size_type n) const noexcept
    {
        auto *p = locked_memory_allocator_allocate(n * sizeof(value_type));
        return reinterpret_cast<value_type *>(p);
    }

    void deallocate(value_type *p, size_type n) const noexcept
    {
        locked_memory_allocator_deallocate(reinterpret_cast<std::byte *>(p), n * sizeof(value_type));
    }
};

} // namespace hi::inline v1
