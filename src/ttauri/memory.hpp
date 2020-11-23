// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>

namespace tt {

template<typename T, typename U>
void memswap(T &dst, U &src) {
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
 * @param src A pointer to an object.
 * @param dst A pointer to allocated memory.
 * @return The dst pointer with the new object who's lifetime was started.
 */
template<typename T>
T *placement_copy(T *src, T *dst)
{
    tt_assume(src != nullptr);
    tt_assume(dst != nullptr);
    tt_assume(src != dst);

    return new (dst) T(*src);
}

/** Copy objects from one memory location to another memory location.
 * This function will placement_copy an array of objects
 * between two memory locations.
 * 
 * The objects may overlap: copying takes place as if the objects were copied
 * to a temporary object array and then the objects were copied from the array to dst.
 */
template<typename T>
void placement_copy(T *src_first, T *src_last, T *dst_first)
{
    tt_assume(src_first != dst_first);
    tt_assume(src_last >= src_first);

    if (src_first < dst_first) {
        auto dst_last = dst_first + (src_last - src_first);

        auto src = src_last;
        auto dst = dst_last;
        while (src != src_first) {
            placement_copy(--src, --dst);
        }

    } else {
        auto src = src_first;
        auto dst = dst_first;
        while (src != src_last) {
            placement_copy(src++, dst++);
        }
    }
}

/** Move an object between two memory locations.
 * This function will move an object from one memory location
 * to another through the use of placement-new. The object
 * in the source memory location is destroyed.
 *
 * If you want to access the object in dst, you should use the return value.
 *
 * @param src A pointer to an object.
 * @param dst A pointer to allocated memory.
 * @return The dst pointer with the new object who's lifetime was started.
 */
template<typename T>
T *placement_move(T *src, T *dst)
{
    tt_assume(src != nullptr);
    tt_assume(dst != nullptr);
    tt_assume(src != dst);

    auto dst_ = new (dst) T(std::move(*src));
    std::destroy_at(src);
    return dst_;
}

/** Move an objects between two memory locations.
 * This function will placement_move an array of objects
 * between two memory locations.
 * 
 * The objects may overlap: copying takes place as if the objects were copied
 * to a temporary object array and then the objects were copied from the array to dst.
 */
template<typename T>
void placement_move(T *src_first, T *src_last, T *dst_first)
{
    tt_assume(src_first != dst_first);
    tt_assume(src_last >= src_first);

    if (src_first < dst_first) {
        auto dst_last = dst_first + (src_last - src_first);

        auto src = src_last;
        auto dst = dst_last;
        while (src != src_first) {
            placement_move(--src, --dst);
        }

    } else {
        auto src = src_first;
        auto dst = dst_first;
        while (src != src_last) {
            placement_move(src++, dst++);
        }
    }
}

template<typename T>
bool is_aligned(T* p){
    return (reinterpret_cast<ptrdiff_t>(p) % std::alignment_of<T>::value) == 0;
}

template<typename R, typename T>
inline R align(T ptr, size_t alignment) noexcept
{
    ttlet byteOffset = reinterpret_cast<ptrdiff_t>(ptr);
    ttlet alignedByteOffset = ((byteOffset + alignment - 1) / alignment) * alignment;

    return reinterpret_cast<R>(alignedByteOffset);
}

/*! Align an end iterator.
* This lowers the end interator so that it the last read is can be done fully.
*/
template<typename R, typename T>
inline R align_end(T ptr, size_t alignment) noexcept
{
    ttlet byteOffset = reinterpret_cast<ptrdiff_t>(ptr);
    ttlet alignedByteOffset = (byteOffset / alignment) * alignment;

    return reinterpret_cast<R>(alignedByteOffset);
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
inline void cleanupWeakPointers(std::unordered_map<K,std::weak_ptr<T>> &v) noexcept
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
inline void cleanupWeakPointers(std::unordered_map<K,std::vector<std::weak_ptr<T>>> &v) noexcept
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
inline std::shared_ptr<Value> try_make_shared(Map &map, Key key, Args... args) {
    std::shared_ptr<Value> value;

    ttlet i = map.find(key);
    if (i == map.end()) {
        value = std::make_shared<Value>(std::forward<Args>(args)...);
        map.insert_or_assign(key, value);
    }
    else {
        value = i->second;
    }
    return value;
}

}
