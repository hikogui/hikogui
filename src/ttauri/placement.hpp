// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "type_traits.hpp"
#include "cast.hpp"
#include "exception.hpp"
#include "check.hpp"
#include <span>

namespace tt {

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
    placement_ptr(std::span<Byte> bytes, ssize_t &offset) {
        Byte *_ptr = bytes.data() + offset;
        offset += ssizeof(T);
        ptr = new(const_cast<std::remove_cv_t<Byte> *>(_ptr)) T;
    }

    ~placement_ptr() {
        std::destroy_at(ptr);
    }

    value_type *operator->() const noexcept {
        return ptr;
    }

    value_type &operator*() const noexcept {
        return *ptr;
    }
};

template<typename T,typename Byte>
placement_ptr<T,Byte> unsafe_make_placement_ptr(std::span<Byte> bytes, ssize_t &offset)
{
    return placement_ptr<T,Byte>(bytes, offset);
}

template<typename T,typename Byte>
placement_ptr<T,Byte> unsafe_make_placement_ptr(std::span<Byte> bytes, ssize_t &&offset = 0)
{
    ssize_t _offset = offset;
    return unsafe_make_placement_ptr<T>(bytes, _offset);
}

template<typename T,typename Byte>
bool check_placement_ptr(std::span<Byte> bytes, ssize_t offset = 0)
{
    return check_alignment<T>(bytes.data()) && (offset + sizeof(T) <= std::size(bytes));
}

template<typename T,typename Byte>
placement_ptr<T,Byte> make_placement_ptr(std::span<Byte> bytes, ssize_t &offset)
{
    tt_parse_check(check_placement_ptr<T>(bytes, offset), "Parsing beyond end of buffer");
    return placement_ptr<T,Byte>(bytes, offset);
}

template<typename T,typename Byte>
placement_ptr<T,Byte> make_placement_ptr(std::span<Byte> bytes, ssize_t &&offset = 0)
{
    ssize_t _offset = offset;
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
    placement_array(std::span<Byte> bytes, ssize_t &offset, ssize_t n) {
        ttlet bytes_ = bytes.data();

        _begin = bytes_ + offset,
        offset += ssizeof(T) * n;
        _end = bytes_ + offset;

        for (ssize_t i = 0; i < n; i++) {
            [[maybe_unused]] auto *ptr = new(const_cast<std::remove_cv_t<Byte> *>(_begin + i * ssizeof(T))) T;
        }
    }

    placement_array(placement_array const &) = delete;
    placement_array(placement_array &&) = delete;
    placement_array &operator=(placement_array const &) = delete;
    placement_array &operator=(placement_array &&) = delete;

    ~placement_array() {
        std::destroy(begin(), end());
    }

    size_t size() const noexcept {
        return static_cast<size_t>(end() - begin());
    }

    bool contains(ssize_t index) const noexcept {
        return index < std::ssize(*this);
    }

    value_type *begin() const noexcept {
        return std::launder(reinterpret_cast<value_type *>(_begin));
    }

    value_type *end() const noexcept {
        return std::launder(reinterpret_cast<value_type *>(_end));
    }

    value_type &operator[](ssize_t offset) const noexcept {
        return *(begin() + offset);
    }
};

template<typename T,typename Byte>
placement_array<T,Byte> unsafe_make_placement_array(std::span<Byte> bytes, ssize_t &offset, ssize_t n)
{
    return placement_array<T,Byte>(bytes, offset, n);
}

template<typename T,typename Byte>
placement_array<T,Byte> unsafe_make_placement_array(std::span<Byte> bytes, ssize_t &&offset, ssize_t n)
{
    ssize_t _offset = offset;
    return unsafe_make_placement_array<T>(bytes, _offset, n);
}

template<typename T,typename Byte>
placement_array<T,Byte> unsafe_make_placement_array(std::span<Byte> bytes, ssize_t &offset)
{
    ttlet n = std::size(bytes) / sizeof(T);
    return unsafe_make_placement_array<T>(bytes, offset, n);
}

template<typename T,typename Byte>
placement_array<T,Byte> unsafe_make_placement_array(std::span<Byte> bytes, ssize_t &&offset = 0)
{
    size_t _offset = offset;
    return unsafe_make_placement_array<T>(bytes, _offset);
}

template<typename T,typename Byte>
bool check_placement_array(std::span<Byte> bytes, ssize_t offset, ssize_t n)
{
    return check_alignment<T>(bytes.data()) && (offset + (n * ssizeof(T)) <= std::ssize(bytes));
}

template<typename T,typename Byte>
bool check_placement_array(std::span<Byte> bytes, ssize_t offset)
{
    return check_alignment<T>(bytes.data());
}

template<typename T,typename Byte>
placement_array<T,Byte> make_placement_array(std::span<Byte> bytes, ssize_t &offset, ssize_t n)
{
    tt_parse_check(check_placement_array<T>(bytes, offset, n), "Parsing beyond end of buffer");
    return placement_array<T,Byte>(bytes, offset, n);
}

template<typename T,typename Byte>
placement_array<T,Byte> make_placement_array(std::span<Byte> bytes, ssize_t &&offset, ssize_t n)
{
    ssize_t _offset = offset;
    return make_placement_array<T>(bytes, _offset, n);
}

template<typename T,typename Byte>
placement_array<T,Byte> make_placement_array(std::span<Byte> bytes, ssize_t &offset)
{
    ttlet n = std::ssize(bytes) / ssizeof(T);
    return make_placement_array<T>(bytes, offset, n);
}

template<typename T,typename Byte>
placement_array<T,Byte> make_placement_array(std::span<Byte> bytes, ssize_t &&offset = 0)
{
    ssize_t _offset = offset;
    return make_placement_array<T>(bytes, _offset);
}

}
