
#include "concepts.hpp"

#pragma once

namespace tt::inline v1 {

void secure_clear(void *ptr, size_t size) noexcept;

constexpr void secure_clear(trivially_copyable auto &object) noexcept
{
    if (std::is_constant_evaluated()) {
        object = std::bit_cast(std::array<char,sizeof(object)>{});
    } else {
        secure_clear(&object, sizeof(object));
    }
}

template<typename It>
constexpr void secure_clear(It first, It last) noexcept
{
    using value_type = decltype(*first);

    if (std::is_constant_evaluated()) {
        for (auto it = first; it != last; ++it) {
            secure_clear(*it);
        }
    } else {
        secure_clear(std::addressof(*first), std::distance(first, last) * sizeof(value_type));
    }
}


constexpr void secure_destroy_at(auto *p)
{
    std::destroy_at(p);
    secure_clear(*p);
}

/** Securely destroy objects.
 *
 * Destroy objects and overwrite its memory with all '1' followed by all '0'.
 *
 * @note Internal allocations of objects will not be securily destroyed.
 * @param first An iterator to the first object.
 * @param last An iterator to beyond the last object.
 */
template<typename It>
constexpr void secure_destroy(It first, It last)
{
    std::destroy(first, last);
    secure_clear(first, last);
}

/** Securely move objects.
 *
 * First calls std::uinitialized_move() then overwrites memory with all '1'
 * followed by all '0'.
 *
 * @param first An iterator to the first object to move
 * @param last An iterator to beyond the last object to move
 * @param d_first An destination iterator to uninitialized memory.
 */
template<typename It, typename OutIt>
void secure_uninitialized_move(It first, It last, OutIt d_first)
{
    std::uninitialized_move(first, last, d_first);
    secure_erase(first, last);
}

}

