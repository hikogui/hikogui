// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <gsl/gsl>

namespace TTauri {

placement_ref

template<typename T>
class casted_span {
    T *_begin;
    T *_end;

    span(T *_begin, T *_end) : _begin(_begin), _end(_end) {}

    ~span() {
        std::destroy(_begin, _end);
    }

    size_t size() const noexcept { return static_cast<size_t>(_end - _begin); }
    T *begin() noexcept { return _begin; }
    T *end() noexcept { return _end; }
    T const * cbegin() const noexcept { return _begin; }
    T const * cend() const noexcept { return _end; }

};

template<typename T>
gsl_suppress(type.1)
inline T &at(gsl::span<std::byte> bytes, size_t offset) noexcept
{
    required_assert(offset + sizeof(T) <= static_cast<size_t>(bytes.size()));
    return *reinterpret_cast<T *>(&bytes[offset]);
}

template<typename T>
gsl_suppress(type.1)
inline T const &at(gsl::span<std::byte const> bytes, size_t offset) noexcept
{
    required_assert(offset + sizeof(T) <= static_cast<size_t>(bytes.size()));
    return *reinterpret_cast<T const *>(&bytes[offset]);
}

template<typename T>
gsl_suppress(type.1)
inline gsl::span<T> make_span(gsl::span<std::byte> bytes, size_t offset, size_t count) noexcept
{
    let size = count * sizeof(T);
    required_assert(offset + size <= static_cast<size_t>(bytes.size()));
    return gsl::span<T>(reinterpret_cast<T *>(&bytes[offset]), count);
}

template<typename T>
gsl_suppress(type.1)
inline gsl::span<T const> make_span(gsl::span<std::byte const> bytes, size_t offset, size_t count) noexcept
{
    let size = count * sizeof(T);
    required_assert(offset + size <= static_cast<size_t>(bytes.size()));
    return gsl::span<T const>(reinterpret_cast<T const *>(&bytes[offset]), count);
}

template<typename T>
gsl_suppress(type.1)
inline gsl::span<T> make_span(gsl::span<std::byte> bytes, size_t offset=0)
{
    let size = static_cast<size_t>(bytes.size());
    let count = size / sizeof(T);
    required_assert(size % sizeof(T) == 0);
    return gsl::span<T>(reinterpret_cast<T *>(&bytes[offset]), count);
}

template<typename T>
gsl_suppress(type.1)
inline gsl::span<T const> make_span(gsl::span<std::byte const> bytes, size_t offset=0)
{
    let size = static_cast<size_t>(bytes.size());
    let count = size / sizeof(T);
    required_assert(size % sizeof(T) == 0);
    return gsl::span<T const>(reinterpret_cast<T const *>(&bytes[offset]), count);
}

}
