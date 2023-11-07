// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <type_traits>
#include <cstddef>
#include <memory>

export module hikogui_memory_secure_memory_allocator;
import hikogui_security;

export namespace hi::inline v1 {

/** Memory allocator which will securely clear the memory when deallocated.
 */
template<typename T>
class secure_memory_allocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;

    template<typename U>
    struct rebind {
        using other = secure_memory_allocator<U>;
    };

    constexpr secure_memory_allocator() noexcept {};

    constexpr secure_memory_allocator(secure_memory_allocator const &other) noexcept {}

    template<typename U>
    constexpr secure_memory_allocator(secure_memory_allocator<U> const &other) noexcept
    {
    }

    [[nodiscard]] value_type *allocate(size_type n) const noexcept
    {
        auto allocator = std::allocator<std::byte>{};
        auto *p = std::allocator_traits<decltype(allocator)>::allocate(allocator, n * sizeof (value_type));
        return reinterpret_cast<value_type *>(p);
    }

    void deallocate(value_type *p, size_type n) const noexcept
    {
        secure_clear(p, n * sizeof (value_type));

        auto allocator = std::allocator<std::byte>{};
        std::allocator_traits<decltype(allocator)>::deallocate(allocator, reinterpret_cast<std::byte *>(p),  n * sizeof (value_type));
    }
};

} // namespace hi::inline v1
