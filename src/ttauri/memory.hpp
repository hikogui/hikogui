// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "assert.hpp"
#include <concepts>
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <type_traits>

namespace tt {

template<typename T, typename U>
void memswap(T &dst, U &src)
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
    tt_axiom(dst != nullptr);
    return new (dst) T(*src);
}

/** Copy objects into a memory location.
 * This function will placement_copy an array of objects
 * to a memory location.
 */
template<typename InputIt, typename T>
void placement_copy(InputIt src_first, InputIt src_last, T *dst_first)
{
    tt_axiom(src_first != dst_first);
    tt_axiom(src_last >= src_first);

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
    tt_axiom(src != nullptr);
    tt_axiom(dst != nullptr);

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
    tt_axiom(src_last >= src_first);

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
    tt_axiom(src_last >= src);

    while (src != src_last) {
        placement_move(src++, dst++);
    }
}

/** Check if a pointer is properly aligned for the object it is pointing at.
 */
template<typename T>
constexpr bool is_aligned(T *p)
{
    return (reinterpret_cast<ptrdiff_t>(p) % std::alignment_of<T>::value) == 0;
}

/** The greatest multiple of alignment less than or equal to value.
 * @param value The unsigned value to round.
 * @param alignment The alignment.
 * @return The greatest multiple of alignment less than or equal to value.
 */
template<std::unsigned_integral T>
constexpr T floor(T value, T alignment) noexcept
{
    return (value / alignment) * alignment;
}

/** The smallest multiple of alignment greater than or equal to value.
 * @param value The unsigned value to round.
 * @param alignment The alignment.
 * @return The smallest multiple of alignment greater than or equal to value.
 */
template<std::unsigned_integral T>
constexpr T ceil(T value, T alignment) noexcept
{
    return floor(value + (alignment - 1), alignment);
}

template<typename T>
constexpr T *ceil(T *ptr, size_t alignment) noexcept
{
    ttlet aligned_byte_offset = ceil(reinterpret_cast<uintptr_t>(ptr), static_cast<uintptr_t>(alignment));
    return reinterpret_cast<T *>(aligned_byte_offset);
}

template<typename T>
constexpr T *floor(T *ptr, size_t alignment) noexcept
{
    ttlet aligned_byte_offset = floor(reinterpret_cast<uintptr_t>(ptr), static_cast<uintptr_t>(alignment));
    return reinterpret_cast<T *>(aligned_byte_offset);
}

template<typename T>
inline void cleanupWeakPointers(std::vector<std::weak_ptr<T>> &v) noexcept
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
inline void cleanupWeakPointers(std::unordered_map<K, std::weak_ptr<T>> &v) noexcept
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
inline void cleanupWeakPointers(std::unordered_map<K, std::vector<std::weak_ptr<T>>> &v) noexcept
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
inline std::shared_ptr<Value> try_make_shared(Map &map, Key key, Args... args)
{
    std::shared_ptr<Value> value;

    ttlet i = map.find(key);
    if (i == map.end()) {
        value = std::make_shared<Value>(std::forward<Args>(args)...);
        map.insert_or_assign(key, value);
    } else {
        value = i->second;
    }
    return value;
}

/** Compress a pointer to a 48 bit unsigned integer.
 *
 * On x64 the virtual address is 48 bits, and the top 16 bit are signed extended from bit 47.
 * The Itanium ABI guaranties allocations are aligned to 16 bytes.
 *
 * On arm the virtual address is 48 or 52 bits, and the top bit are sign extended. The top 8
 * bit may be ignored by the CPU to implement tagged addressing, of which the bottom 4 bits
 * of those may used as a hardware-key. The ARM64 ABI requires the stack to be aligned to
 * 16 bytes, I am expecting heap allocation to be also aligned to 16 bytes.
 */
inline uint64_t ptr_to_uint48(auto *ptr) noexcept
{
    tt_axiom(static_cast<uint64_t>(ptr) % 16 == 0);

    if constexpr (processor::current == processor::x64) {
        // Only the bottom 48 bits are needed.
        tt_axiom(
            (static_cast<uint64_t>(ptr) & 0xffff'8000'0000'0000) == 0 ||
            (static_cast<uint64_t>(ptr) & 0xffff'8000'0000'0000) == 0xffff'8000'0000'0000);
        return (static_cast<uint64_t>(ptr) << 16) >> 16;

    } else if constexpr (processor::current == processor::arm) {
        // The top 8 bits may contain a tag.
        tt_axiom(
            (static_cast<uint64_t>(ptr) & 0x00ff'8000'0000'0000) == 0 ||
            (static_cast<uint64_t>(ptr) & 0x00ff'8000'0000'0000) == 0x00ff'8000'0000'0000);

        // Take the 4 sign bits + 44 msb bits of address.
        auto u64 = (static_cast<uint64_t>(ptr) << 12) >> 16;

        // Extract the 4 bit key.
        auto key = (static_cast<uint64_t>(ptr) >> 56) << 44;

        // XOR the key with the sign bits in the upper part of the 48 bit result.
        return key ^ u64;

    } else {
        tt_static_no_default();
    }
}

/** Uncompress a 48 bit unsigned integer into a pointer.
 *
 * On x64 the virtual address is 48 bits, and the top 16 bit are signed extended from bit 47.
 * The Itanium ABI guaranties allocations are aligned to 16 bytes.
 *
 * On arm the virtual address is 48 or 52 bits, and the top bit are sign extended. The top 8
 * bit may be ignored by the CPU to implement tagged addressing, of which the bottom 4 bits
 * of those may used as a hardware-key. The ARM64 ABI requires the stack to be aligned to
 * 16 bytes, I am expecting heap allocation to be also aligned to 16 bytes.
 */
template<typename T>
T *uint48_to_ptr(uint64_t x) noexcept
{
    tt_axiom((x >> 48) == 0);

    if constexpr (processor::current == processor::x64) {
        // Shift the upper bits away and sign extend the upper 16 bits.
        auto i64 = (static_cast<int64_t>(x) << 16) >> 16;
        return reinterpret_cast<T *>(i64);

    } else if constexpr (processor::current == processor::arm) {
        // Get 4 bit key (xor-ed with the sign bits).
        auto key = (static_cast<uint64_t>(x) >> 44) << 56;

        // Sign extend the address and make the bottom 4 bits zero.
        auto i64 = (static_cast<int64_t>(x) << 20) >> 16;

        // Add the original key by XOR with the sign.
        return reinterpret_cast<T *>(key ^ static_cast<uint64_t>(i64));

    } else {
        tt_static_no_default();
    }
}

} // namespace tt
