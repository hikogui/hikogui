// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/type_traits.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include <gsl/gsl>

namespace TTauri {

template<typename T>
inline bool check_alignment(void const *ptr) {
    return reinterpret_cast<ptrdiff_t>(ptr) % alignof(T) == 0;
}


template<typename T,typename Byte>
class placement_ptr {
    static_assert(
        std::is_same_v<std::remove_cv_t<Byte>,std::byte> ||
        std::is_same_v<std::remove_cv_t<Byte>,char> ||
        std::is_same_v<std::remove_cv_t<Byte>,unsigned char> ||
        std::is_same_v<std::remove_cv_t<Byte>,signed char>,
        "Byte must be a byte type"
    );
    static_assert(std::is_trivially_constructible_v<T>);
    static_assert(!std::is_const_v<Byte> || std::is_trivially_destructible_v<T>);

    using value_type = copy_cv_t<T,Byte>;
    value_type *ptr;
    
public:
    force_inline placement_ptr(gsl::span<Byte> bytes, size_t &offset) {
        Byte *_ptr = bytes.data() + offset;
        offset += sizeof(T);
        ptr = new(const_cast<std::remove_cv_t<Byte> *>(_ptr)) T;
    }

    force_inline ~placement_ptr() {
        std::destroy_at(ptr);
    }

    force_inline value_type *operator->() const noexcept {
        return ptr;
    }

    force_inline value_type &operator*() const noexcept {
        return *ptr;
    }
};

template<typename T,typename Byte>
force_inline placement_ptr<T,Byte> unsafe_make_placement_ptr(gsl::span<Byte> bytes, size_t &offset)
{
    return placement_ptr<T,Byte>(bytes, offset);
}

template<typename T,typename Byte>
force_inline placement_ptr<T,Byte> unsafe_make_placement_ptr(gsl::span<Byte> bytes, size_t &&offset = 0)
{
    size_t _offset = offset;
    return unsafe_make_placement_ptr<T>(bytes, _offset);
}

template<typename T,typename Byte>
force_inline bool check_placement_ptr(gsl::span<Byte> bytes, size_t offset = 0)
{
    return check_alignment<T>(bytes.data()) && (offset + sizeof(T) <= usize(bytes));
}

template<typename T,typename Byte>
force_inline placement_ptr<T,Byte> make_placement_ptr(gsl::span<Byte> bytes, size_t &offset)
{
    parse_assert(check_placement_ptr<T>(bytes, offset));
    return placement_ptr<T,Byte>(bytes, offset);
}

template<typename T,typename Byte>
force_inline placement_ptr<T,Byte> make_placement_ptr(gsl::span<Byte> bytes, size_t &&offset = 0)
{
    size_t _offset = offset;
    return make_placement_ptr<T>(bytes, _offset);
}

template<typename T, typename Byte>
class placement_array {
    static_assert(
        std::is_same_v<std::remove_cv_t<Byte>,std::byte> ||
        std::is_same_v<std::remove_cv_t<Byte>,char> ||
        std::is_same_v<std::remove_cv_t<Byte>,unsigned char> ||
        std::is_same_v<std::remove_cv_t<Byte>,signed char>,
        "Byte must be a byte type"
    );
    static_assert(std::is_trivially_constructible_v<T>);
    static_assert(!std::is_const_v<T> || std::is_trivially_destructible_v<T>);

    using value_type = copy_cv_t<T,Byte>;

    Byte *_begin;
    Byte *_end;

public:
    force_inline placement_array(gsl::span<Byte> bytes, size_t &offset, size_t n) {
        let bytes_ = bytes.data();

        _begin = bytes_ + offset,
        offset += sizeof(T) * n;
        _end = bytes_ + offset;

        for (size_t i = 0; i < n; i++) {
            [[maybe_unused]] auto *ptr = new(const_cast<std::remove_cv_t<Byte> *>(_begin + i * sizeof(T))) T;
        }
    }

    placement_array(placement_array const &) = delete;
    placement_array(placement_array &&) = delete;
    placement_array &operator=(placement_array const &) = delete;
    placement_array &operator=(placement_array &&) = delete;

    force_inline ~placement_array() {
        std::destroy(begin(), end());
    }

    force_inline size_t size() const noexcept {
        return static_cast<size_t>(end() - begin());
    }

    force_inline bool contains(size_t index) const noexcept {
        return index < size();
    }

    force_inline value_type *begin() const noexcept {
        return std::launder(reinterpret_cast<value_type *>(_begin));
    }

    force_inline value_type *end() const noexcept {
        return std::launder(reinterpret_cast<value_type *>(_end));
    }

    force_inline value_type &operator[](size_t offset) const noexcept {
        return *(begin() + offset);
    }
};

template<typename T,typename Byte>
force_inline placement_array<T,Byte> unsafe_make_placement_array(gsl::span<Byte> bytes, size_t &offset, size_t n)
{
    return placement_array<T,Byte>(bytes, offset, n);
}

template<typename T,typename Byte>
force_inline placement_array<T,Byte> unsafe_make_placement_array(gsl::span<Byte> bytes, size_t &&offset, size_t n)
{
    size_t _offset = offset;
    return unsafe_make_placement_array<T>(bytes, _offset, n);
}

template<typename T,typename Byte>
force_inline placement_array<T,Byte> unsafe_make_placement_array(gsl::span<Byte> bytes, size_t &offset)
{
    let n = usize(bytes) / sizeof(T);
    return unsafe_make_placement_array<T>(bytes, offset, n);
}

template<typename T,typename Byte>
force_inline placement_array<T,Byte> unsafe_make_placement_array(gsl::span<Byte> bytes, size_t &&offset = 0)
{
    size_t _offset = offset;
    return unsafe_make_placement_array<T>(bytes, _offset);
}

template<typename T,typename Byte>
force_inline bool check_placement_array(gsl::span<Byte> bytes, size_t offset, size_t n)
{
    return check_alignment<T>(bytes.data()) && (offset + (n * sizeof(T)) <= usize(bytes));
}

template<typename T,typename Byte>
force_inline bool check_placement_array(gsl::span<Byte> bytes, size_t offset)
{
    return check_alignment<T>(bytes.data());
}

template<typename T,typename Byte>
force_inline placement_array<T,Byte> make_placement_array(gsl::span<Byte> bytes, size_t &offset, size_t n)
{
    parse_assert(check_placement_array<T>(bytes, offset, n));
    return placement_array<T,Byte>(bytes, offset, n);
}

template<typename T,typename Byte>
force_inline placement_array<T,Byte> make_placement_array(gsl::span<Byte> bytes, size_t &&offset, size_t n)
{
    size_t _offset = offset;
    return make_placement_array<T>(bytes, _offset, n);
}

template<typename T,typename Byte>
force_inline placement_array<T,Byte> make_placement_array(gsl::span<Byte> bytes, size_t &offset)
{
    let n = static_cast<size_t>(usize(bytes) / sizeof(T));
    return make_placement_array<T>(bytes, offset, n);
}

template<typename T,typename Byte>
force_inline placement_array<T,Byte> make_placement_array(gsl::span<Byte> bytes, size_t &&offset = 0)
{
    size_t _offset = offset;
    return make_placement_array<T>(bytes, _offset);
}

}
