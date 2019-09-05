// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>

namespace TTauri {

template<typename T, typename U>
void memswap(T &dst, U &src) {
    static_assert(sizeof(T) == sizeof(U), "memswap requires both objects of equal size");
    std::byte tmp[sizeof(T)];
    memcpy(tmp, &src, sizeof(T));
    memcpy(&src, &dst, sizeof(U));
    memcpy(&dst, tmp, sizeof(T));
}

template<typename R, typename T>
gsl_suppress3(type.1,26487,lifetime.4)
    inline R align(T ptr, size_t alignment) noexcept
{
    let byteOffset = reinterpret_cast<ptrdiff_t>(ptr);
    let alignedByteOffset = ((byteOffset + alignment - 1) / alignment) * alignment;

    return reinterpret_cast<R>(alignedByteOffset);
}

/*! Align an end iterator.
* This lowers the end interator so that it the last read is can be done fully.
*/
template<typename R, typename T>
gsl_suppress5(f.23,bounds.3,type.1,26487,lifetime.4)
    inline R align_end(T ptr, size_t alignment) noexcept
{
    let byteOffset = reinterpret_cast<ptrdiff_t>(ptr);
    let alignedByteOffset = (byteOffset / alignment) * alignment;

    return reinterpret_cast<R>(alignedByteOffset);
}

template <class To, class From>
typename std::enable_if<(sizeof(To) == sizeof(From)) && std::is_trivially_copyable<From>::value && std::is_trivial<To>::value,
    // this implementation requires that To is trivially default constructible
    To>::type
    // constexpr support needs compiler magic
    bit_cast(const From &src) noexcept
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

}
