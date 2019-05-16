// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "logging.hpp"
#include <boost/throw_exception.hpp>
#include <gsl/gsl>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <functional>
#include <algorithm>
#include <thread>
#include <atomic>

namespace TTauri {

    struct NotImplementedError : virtual boost::exception, virtual std::exception {};
    struct OutOfBoundsError : virtual boost::exception, virtual std::exception {};

#define let auto const

template<typename T, bool result = std::is_same<decltype(((T *)nullptr)->initialize()), void>::value>
constexpr bool hasInitializeHelper(int)
{
    return result;
}

template<typename T>
constexpr bool hasInitializeHelper(...)
{
    return false;
}

template<typename T>
constexpr bool hasInitialize()
{
    return hasInitializeHelper<T>(0);
}

template<typename T, typename... Args, typename std::enable_if_t<TTauri::hasInitialize<T>(), int> = 0>
inline std::shared_ptr<T> make_shared(Args... args)
{
    auto tmp = std::make_shared<T>(args...);
    tmp->initialize();
    return tmp;
}

template<typename T, typename... Args, typename std::enable_if_t<!TTauri::hasInitialize<T>(), int> = 0>
inline std::shared_ptr<T> make_shared(Args... args)
{
    return std::make_shared<T>(args...);
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
inline gsl::span<T> make_span(gsl::span<std::byte> bytes, size_t offset, size_t count)
{
    size_t size = count * sizeof(T);

    if (offset + size > static_cast<size_t>(bytes.size())) {
        BOOST_THROW_EXCEPTION(OutOfBoundsError());
    }

    T* ptr = reinterpret_cast<T *>(&bytes[offset]);
    return gsl::span<T>(ptr, count);
}

inline constexpr size_t align(size_t offset, size_t alignment) 
{
    return ((offset + alignment - 1) / alignment) * alignment;
}

inline constexpr uint32_t fourcc(char *txt)
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

inline std::vector<std::string> split(std::string haystack, char needle)
{
    std::vector<std::string> r;

    size_t offset = 0;
    size_t pos = haystack.find('.', offset);
    while (pos != haystack.npos) {
        r.push_back(haystack.substr(offset, pos - offset));

        offset = pos + 1;
        pos = haystack.find('.', offset);
    }

    r.push_back(haystack.substr(offset, haystack.size() - offset));
    return r;
}

template<typename T, typename U>
inline std::shared_ptr<T> lock_dynamic_cast(const std::weak_ptr<U> &x)
{
    return std::dynamic_pointer_cast<T>(x.lock());
}

struct GetSharedCastError : virtual boost::exception, virtual std::exception {};

template<typename T, typename std::enable_if_t<std::is_constructible_v<T>, int> = 0>
inline std::shared_ptr<T> get_singleton()
{
    if (!T::singleton) {
        T::singleton = std::make_shared<T>();
    }

    auto tmpCastedShared = std::dynamic_pointer_cast<T>(T::singleton);
    if (!tmpCastedShared) {
        BOOST_THROW_EXCEPTION(GetSharedCastError());
    }

    return tmpCastedShared;
}

template<typename T, typename std::enable_if_t<!std::is_constructible_v<T>, int> = 0>
inline std::shared_ptr<T> get_singleton()
{
    auto tmpCastedShared = std::dynamic_pointer_cast<T>(T::singleton);
    if (!tmpCastedShared) {
        BOOST_THROW_EXCEPTION(GetSharedCastError());
    }

    return tmpCastedShared;
}

struct MakeSharedNotNull : virtual boost::exception, virtual std::exception {};

template<typename T, typename... Args>
inline decltype(auto) make_singleton(Args... args)
{
    if (T::singleton) {
        BOOST_THROW_EXCEPTION(MakeSharedNotNull());
    }

    T::singleton = std::make_shared<T>(args...);
    return T::singleton->initialize();
}

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
        auto const m = middle(begin, end);

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
    std::transform(input.begin(), input.end(), std::back_inserter(result), operation);
    return result;
}

template<typename T, size_t N, typename F>
constexpr std::array<T, N> generate_array(F operation)
{
    std::array<T, N> a;

    for (size_t i = 0; i < N; i++) {
        a[i] = operation(i);
    }

    return a;
}

template<typename T, typename F>
inline void erase_if(T &v, F operation)
{
    while (true) {
        auto const i = std::find_if(v.begin(), v.end(), operation);
        if (i == v.end()) {
            return;
        }
        v.erase(i);
    }
}

}
