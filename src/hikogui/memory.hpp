// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "assert.hpp"
#include "concepts.hpp"
#include "math.hpp"
#include <concepts>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <type_traits>
#include <string.h>

hi_warning_push();
// C26474: Don't cast between pointer types when the conversion could be implicit (type.1).
// False positive, template with potential two different pointer types.
hi_warning_ignore_msvc(26474);

namespace hi::inline v1 {

[[nodiscard]] bool equal_ptr(auto *p1, auto *p2) noexcept
{
    return static_cast<void *>(p1) == static_cast<void *>(p2);
}

template<typename T, typename U>
void memswap(T& dst, U& src)
{
    static_assert(sizeof(T) == sizeof(U), "memswap requires both objects of equal size");
    std::byte tmp[sizeof(T)];
    memcpy(tmp, &src, sizeof(T));
    memcpy(&src, &dst, sizeof(U));
    memcpy(&dst, tmp, sizeof(T));
}

/** Copy an object to another memory locations.
 * This function will copy an object located in one memory location
 * to another through the use of placement-new.
 *
 * If you want to access the object in dst, you should use the return value.
 *
 * @param src An interator to an object.
 * @param dst A pointer to allocated memory.
 * @return The dst pointer with the new object who's lifetime was started.
 */
template<typename InputIt, typename T>
T *placement_copy(InputIt src, T *dst)
{
    hi_axiom_not_null(dst);
    return new (dst) T(*src);
}

/** Copy objects into a memory location.
 * This function will placement_copy an array of objects
 * to a memory location.
 */
template<typename InputIt, typename T>
void placement_copy(InputIt src_first, InputIt src_last, T *dst_first)
{
    hi_axiom(src_first != dst_first);
    hi_axiom(src_last >= src_first);

    auto src = src_first;
    auto dst = dst_first;
    while (src != src_last) {
        placement_copy(src++, dst++);
    }
}

/** Move an object between two memory locations.
 * This function will move an object from one memory location
 * to another through the use of placement-new. The object
 * in the source memory location is destroyed.
 *
 * If you want to access the object in dst, you should use the return value.
 * It is undefined behavior when src and dst point to the same object.
 * It is undefined behavior if either or both src and dst are nullptr.
 *
 * @param src A pointer to an object.
 * @param dst A pointer to allocated memory.
 * @return The dst pointer with the new object who's lifetime was started.
 */
template<typename T>
T *placement_move(T *src, T *dst)
{
    hi_axiom_not_null(src);
    hi_axiom_not_null(dst);

    auto dst_ = new (dst) T(std::move(*src));
    std::destroy_at(src);
    return dst_;
}

/** Move an objects between two memory locations.
 * This function will placement_move an array of objects
 * between two memory locations.
 *
 * It is undefined behavior when src_first and dst_first are not part of the same array.
 *
 * The objects may overlap: copying takes place as if the objects were copied
 * to a temporary object array and then the objects were copied from the array to dst.
 */
template<typename T>
void placement_move_within_array(T *src_first, T *src_last, T *dst_first)
{
    hi_axiom(src_last >= src_first);

    if (src_first < dst_first) {
        auto dst_last = dst_first + (src_last - src_first);

        auto src = src_last;
        auto dst = dst_last;
        while (src != src_first) {
            placement_move(--src, --dst);
        }

    } else if (src_first > dst_first) {
        auto src = src_first;
        auto dst = dst_first;
        while (src != src_last) {
            placement_move(src++, dst++);
        }

    } else {
        // When src_first and dst_first are equal then no movement is necessary.
        ;
    }
}

/** Move an objects between two memory locations.
 * This function will placement_move an array of objects
 * between two memory locations.
 *
 * WARNING: if moving objects within an array use `placement_move_within_array`
 * to handle overlapping regions.
 */
template<typename T>
void placement_move(T *src, T *src_last, T *dst)
{
    hi_axiom(src_last >= src);

    while (src != src_last) {
        placement_move(src++, dst++);
    }
}

/** Construct a set of objects.
 */
template<typename It, typename... Args>
void construct(It first, It last, Args const&...args)
{
    for (auto it = first; it != last; ++it) {
        std::construct_at(std::addressof(*it), args...);
    }
}

/** Check if a pointer is properly aligned for the object it is pointing at.
 */
template<typename T>
constexpr bool is_aligned(T *p)
{
    return (reinterpret_cast<ptrdiff_t>(p) % std::alignment_of<T>::value) == 0;
}



template<typename T>
constexpr T *ceil(T *ptr, std::size_t alignment) noexcept
{
    hilet aligned_byte_offset = ceil(reinterpret_cast<uintptr_t>(ptr), static_cast<uintptr_t>(alignment));
    return reinterpret_cast<T *>(aligned_byte_offset);
}

template<typename T>
constexpr T *floor(T *ptr, std::size_t alignment) noexcept
{
    hilet aligned_byte_offset = floor(reinterpret_cast<uintptr_t>(ptr), static_cast<uintptr_t>(alignment));
    return reinterpret_cast<T *>(aligned_byte_offset);
}

/** Advance a pointer by a number of bytes.
 *
 * @note It is undefined behavior for ptr to be nullptr
 * @param ptr The pointer to advance.
 * @param distance The number of bytes to advance the pointer, may be negative.
 */
inline void *advance_bytes(void *ptr, std::ptrdiff_t distance) noexcept
{
    hi_axiom_not_null(ptr);
    return static_cast<char *>(ptr) + distance;
}

/** Advance a pointer by a number of bytes.
 *
 * @note It is undefined behavior for ptr to be nullptr
 * @param ptr The pointer to advance.
 * @param distance The number of bytes to advance the pointer, may be negative.
 */
inline void const *advance_bytes(void const *ptr, std::ptrdiff_t distance) noexcept
{
    hi_axiom_not_null(ptr);
    return static_cast<char const *>(ptr) + distance;
}

template<typename T>
inline void cleanupWeakPointers(std::vector<std::weak_ptr<T>>& v) noexcept
{
    auto i = v.begin();
    while (i != v.end()) {
        if (i->expired()) {
            i = v.erase(i);
        } else {
            i++;
        }
    }
}

template<typename K, typename T>
inline void cleanupWeakPointers(std::unordered_map<K, std::weak_ptr<T>>& v) noexcept
{
    auto i = v.begin();
    while (i != v.end()) {
        if (i->second.expired()) {
            i = v.erase(i);
        } else {
            i++;
        }
    }
}

template<typename K, typename T>
inline void cleanupWeakPointers(std::unordered_map<K, std::vector<std::weak_ptr<T>>>& v) noexcept
{
    auto i = v.begin();
    while (i != v.end()) {
        cleanupWeakPointers(i->second);
        if (i->second.size() == 0) {
            i = v.erase(i);
        } else {
            i++;
        }
    }
}

template<typename Value, typename Map, typename Key, typename... Args>
inline std::shared_ptr<Value> try_make_shared(Map& map, Key key, Args... args)
{
    std::shared_ptr<Value> value;

    hilet i = map.find(key);
    if (i == map.end()) {
        value = std::make_shared<Value>(std::forward<Args>(args)...);
        map.insert_or_assign(key, value);
    } else {
        value = i->second;
    }
    return value;
}

/** Make an unaligned load of an unsigned integer.
 */
template<numeric T>
[[nodiscard]] hi_force_inline constexpr T load(uint8_t const *src) noexcept
{
    auto r = T{};

    if (not std::is_constant_evaluated()) {
        // MSVC, clang and gcc are able to optimize this fully on x86-64.
        std::memcpy(&r, src, sizeof(T));
        return r;
    }

    if constexpr (std::endian::native == std::endian::little) {
        for (auto i = sizeof(T); i != 0; --i) {
            r <<= 8;
            r |= src[i-1];
        }
    } else {
        for (auto i = 0; i != sizeof(T); ++i) {
            r <<= 8;
            r |= src[i];
        }
    }
    return r;
}

template<numeric T>
[[nodiscard]] hi_force_inline constexpr void store(T src, uint8_t const *dst) noexcept
{
    using unsigned_type = std::make_unsigned_t<T>;

    hilet src_ = static_cast<unsigned_type>(src);

    if (not std::is_constant_evaluated()) {
#if HI_COMPILER == HI_CC_MSVC
        *reinterpret_cast<__unaligned unsigned_type const *>(dst) = src_;
        return;
#else
        std::memcpy(dst, &src, sizeof(T));
        return;
#endif
    }

    if constexpr (std::endian::native == std::endian::little) {
        for (auto i = 0; i != sizeof(T); ++i) {
            dst[i] = static_cast<uint8_t>(src_);
            src_ >>= 8;
        }
    } else {
        for (auto i = sizeof(T); i != 0; --i) {
            dst[i] = static_cast<uint8_t>(src_);
            src_ >>= 8;
        }
    }
}

template<numeric T>
[[nodiscard]] hi_force_inline constexpr void store_or(T src, uint8_t *dst) noexcept
{
    using unsigned_type = std::make_unsigned_t<T>;

    auto src_ = static_cast<unsigned_type>(src);

    if (not std::is_constant_evaluated()) {
#if HI_COMPILER == HI_CC_MSVC
        *reinterpret_cast<__unaligned unsigned_type *>(dst) |= src_;
        return;
#else
        decltype(src_) tmp;
        std::memcpy(&tmp, dst, sizeof(T));
        tmp |= src_;
        std::memcpy(dst, &tmp, sizeof(T));
#endif
    }

    if constexpr (std::endian::native == std::endian::little) {
        for (auto i = 0; i != sizeof(T); ++i) {
            dst[i] |= static_cast<uint8_t>(src_);
            src_ >>= 8;
        }
    } else {
        for (auto i = sizeof(T); i != 0; --i) {
            dst[i] |= static_cast<uint8_t>(src_);
            src_ >>= 8;
        }
    }
}

} // namespace hi::inline v1

hi_warning_pop();
