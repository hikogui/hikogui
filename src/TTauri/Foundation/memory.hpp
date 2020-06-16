// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>

namespace tt {

template<typename T, typename U>
tt_force_inline void memswap(T &dst, U &src) {
    static_assert(sizeof(T) == sizeof(U), "memswap requires both objects of equal size");
    std::byte tmp[sizeof(T)];
    memcpy(tmp, &src, sizeof(T));
    memcpy(&src, &dst, sizeof(U));
    memcpy(&dst, tmp, sizeof(T));
}

template<typename T>
tt_force_inline bool is_aligned(T* p){
    return (reinterpret_cast<ptrdiff_t>(p) % std::alignment_of<T>::value) == 0;
}

template<typename R, typename T>
gsl_suppress3(type.1,26487,lifetime.4)
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
gsl_suppress5(f.23,bounds.3,type.1,26487,lifetime.4)
    inline R align_end(T ptr, size_t alignment) noexcept
{
    ttlet byteOffset = reinterpret_cast<ptrdiff_t>(ptr);
    ttlet alignedByteOffset = (byteOffset / alignment) * alignment;

    return reinterpret_cast<R>(alignedByteOffset);
}

// this implementation requires that To is trivially default constructible
template<typename To, typename From>
std::enable_if_t<(sizeof(To) == sizeof(From)) && std::is_trivially_copyable_v<From> && std::is_trivial_v<To>,To>
bit_cast(From const &src) noexcept
{
    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
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
