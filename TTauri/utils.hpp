// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "exceptions.hpp"
#include <boost/throw_exception.hpp>
#include <gsl/gsl>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <functional>
#include <algorithm>
#include <thread>
#include <atomic>
#include <type_traits>
#include <unordered_map>

namespace TTauri {


template<typename T>
inline T &get_singleton()
{
    static auto x = std::make_unique<T>();
    return *x;
}

template<typename T>
inline T &at(gsl::span<std::byte> bytes, size_t offset)
{
    if (offset + sizeof(T) > static_cast<size_t>(bytes.size())) {
        BOOST_THROW_EXCEPTION(OutOfBoundsError());
    }

    T *ptr = reinterpret_cast<T *>(&bytes[offset]);
    return *ptr;
}

template<typename T>
inline T const &at(gsl::span<std::byte const> bytes, size_t offset)
{
    if (offset + sizeof(T) > static_cast<size_t>(bytes.size())) {
        BOOST_THROW_EXCEPTION(OutOfBoundsError());
    }

    return *reinterpret_cast<T const *>(&bytes[offset]);
}

template<typename T>
inline gsl::span<T> make_span(gsl::span<std::byte> bytes, size_t offset, size_t count)
{
    size_t size = count * sizeof(T);

    if (offset + size > static_cast<size_t>(bytes.size())) {
        BOOST_THROW_EXCEPTION(OutOfBoundsError());
    }

    T *ptr = reinterpret_cast<T *>(&bytes[offset]);
    return gsl::span<T>(ptr, count);
}

template<typename T>
inline gsl::span<T const> make_span(gsl::span<std::byte const> bytes, size_t offset, size_t count)
{
    size_t size = count * sizeof(T);

    if (offset + size > static_cast<size_t>(bytes.size())) {
        BOOST_THROW_EXCEPTION(OutOfBoundsError());
    }

    T const *ptr = reinterpret_cast<T const *>(&bytes[offset]);
    return gsl::span<T const>(ptr, count);
}

template<typename T>
inline gsl::span<T> make_span(gsl::span<std::byte> bytes, size_t offset=0)
{
    size_t count = bytes.size() / sizeof(T);

    T *ptr = reinterpret_cast<T *>(&bytes[offset]);
    return gsl::span<T>(ptr, count);
}

template<typename T>
inline gsl::span<T const> make_span(gsl::span<std::byte const> bytes, size_t offset=0)
{
    size_t count = bytes.size() / sizeof(T);

    T const* ptr = reinterpret_cast<T const *>(&bytes[offset]);
    return gsl::span<T const>(ptr, count);
}


template<typename R, typename T>
inline R align(T ptr, size_t alignment) 
{
    let byteOffset = reinterpret_cast<ptrdiff_t>(ptr);
    let alignedByteOffset = ((byteOffset + alignment - 1) / alignment) * alignment;
    return reinterpret_cast<R>(alignedByteOffset);
}

/*! Align an end iterator.
 * This lowers the end interator so that it the last read is can be done fully.
 */
template<typename R, typename T>
inline R align_end(T ptr, size_t alignment)
{
    let byteOffset = reinterpret_cast<ptrdiff_t>(ptr);
    let alignedByteOffset = (byteOffset / alignment) * alignment;
    return reinterpret_cast<R>(alignedByteOffset);
}

inline constexpr uint32_t fourcc(char const txt[5])
{
    return (
        (static_cast<uint32_t>(txt[0]) << 24) |
        (static_cast<uint32_t>(txt[1]) << 16) |
        (static_cast<uint32_t>(txt[2]) <<  8) |
        static_cast<uint32_t>(txt[3])
   );
}

inline std::string fourcc_to_string(uint32_t x)
{
    char c_str[5];
    c_str[0] = static_cast<char>((x >> 24) & 0xff);
    c_str[1] = static_cast<char>((x >> 16) & 0xff);
    c_str[2] = static_cast<char>((x >> 8) & 0xff);
    c_str[3] = static_cast<char>(x & 0xff);
    c_str[4] = 0;

    return {c_str};
}

template<typename T>
inline typename T::value_type pop_back(T &v)
{
    typename T::value_type x = std::move(v.back());
    v.pop_back();
    return x;
}

template <typename T>
inline std::vector<T> split(T haystack, char needle)
{
    std::vector<T> r;

    size_t offset = 0;
    size_t pos = haystack.find(needle, offset);
    while (pos != haystack.npos) {
        r.push_back(haystack.substr(offset, pos - offset));

        offset = pos + 1;
        pos = haystack.find(needle, offset);
    }

    r.push_back(haystack.substr(offset, haystack.size() - offset));
    return r;
}

struct GetSharedCastError : virtual boost::exception, virtual std::exception {};
struct MakeSharedNotNull : virtual boost::exception, virtual std::exception {};

template<typename T>
inline std::enable_if_t<!std::is_pointer_v<T>, T> middle(T begin, T end)
{
    return begin + std::distance(begin, end) / 2;
}

template<typename T>
inline std::enable_if_t<std::is_pointer_v<T>, T> middle(T begin, T end)
{
    return reinterpret_cast<T>((reinterpret_cast<intptr_t>(begin) + reinterpret_cast<intptr_t>(end)) / 2);;
}

template<typename T, typename U>
inline T binary_nearest_find(T begin, T end, U value)
{
    while (begin < end) {
        let m = middle(begin, end);

        if (value > *m) {
            begin = m + 1;
        } else if (value < *m) {
            end = m;
        } else {
            return m;
        }
    }
    return begin;
}

template<typename T, typename U, typename F>
inline T transform(const U &input, F operation)
{
    T result = {};
    result.reserve(input.size());
    std::transform(input.begin(), input.end(), std::back_inserter(result), operation);
    return result;
}

template<typename T, size_t N, typename F>
constexpr std::array<T, N> generate_array(F operation)
{
    std::array<T, N> a{};

    for (size_t i = 0; i < N; i++) {
        a[i] = operation(i);
    }

    return a;
}

template<typename T, typename F>
inline void erase_if(T &v, F operation)
{
    while (true) {
        let i = std::find_if(v.begin(), v.end(), operation);
        if (i == v.end()) {
            return;
        }
        v.erase(i);
    }
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

inline char nibble_to_char(uint8_t nibble)
{
    if (nibble <= 9) {
        return '0' + nibble;
    } else if (nibble <= 15) {
        return 'a' + nibble - 10;
    } else {
        no_default;
    }
}

inline uint8_t char_to_nibble(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return (c - 'a') + 10;
    } else if (c >= 'A' && c <= 'F') {
        return (c - 'A') + 10;
    } else {
        BOOST_THROW_EXCEPTION(ParseError("Could not parse hexadecimal digit")
            << errinfo_parse_string(std::string(1, c))
        );
    }
}

template<typename T>
inline void cleanupWeakPointers(std::vector<std::weak_ptr<T>> &v)
{
    using iterator = typename std::remove_reference_t<decltype(v)>::const_iterator;
    auto expiredIterators = std::vector<iterator>{};

    for (auto i = v.begin(); i != v.end(); i++) {
        if (i->expired()) {
            expiredIterators.push_back(i);
        }
    }

    for (let &i : expiredIterators) {
        v.erase(i); 
    }
}

template<typename K, typename T>
inline void cleanupWeakPointers(std::unordered_map<K,std::weak_ptr<T>> &v)
{
    using iterator = typename std::remove_reference_t<decltype(v)>::const_iterator;
    auto expiredIterators = std::vector<iterator>{};

    for (auto i = v.begin(); i != v.end(); i++) {
        if (i->second.expired() == 0) {
            expiredIterators.push_back(i);
        }
    }

    for (let &i : expiredIterators) {
        v.erase(i); 
    }
}

template<typename K, typename T>
inline void cleanupWeakPointers(std::unordered_map<K,std::vector<std::weak_ptr<T>>> &v)
{
    using iterator = typename std::remove_reference_t<decltype(v)>::const_iterator;
    auto expiredIterators = std::vector<iterator>{};

    for (auto i = v.begin(); i != v.end(); i++) {
        cleanupWeakPointers(i->second);
        if (i->second.size() == 0) {
            expiredIterators.push_back(i);
        }
    }

    for (let &i : expiredIterators) {
        v.erase(i); 
    }
}

}
