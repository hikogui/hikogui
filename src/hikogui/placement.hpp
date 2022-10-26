// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "type_traits.hpp"
#include "cast.hpp"
#include "exception.hpp"
#include "check.hpp"
#include <span>

hi_warning_push();
// C26492: Don't use const_cast to cast away const or volatile (type.3).
// placement new requires non-const pointer, even if the non-constructor initializer doesn't
// modify the memory.
hi_warning_ignore_msvc(26492)

namespace hi::inline v1 {

template<typename T>
inline bool check_alignment(void const *ptr) noexcept
{
    return std::bit_cast<uintptr_t>(ptr) % alignof(T) == 0;
}

template<typename T, typename Byte>
class placement_ptr {
    static_assert(
        std::is_same_v<std::remove_cv_t<Byte>, std::byte> || std::is_same_v<std::remove_cv_t<Byte>, char> ||
            std::is_same_v<std::remove_cv_t<Byte>, unsigned char> || std::is_same_v<std::remove_cv_t<Byte>, signed char>,
        "Byte must be a byte type");
    static_assert(std::is_trivially_constructible_v<T>);
    static_assert(!std::is_const_v<Byte> || std::is_trivially_destructible_v<T>);

    using value_type = copy_cv_t<T, Byte>;
    value_type *ptr;

public:
    placement_ptr(std::span<Byte> bytes, std::size_t& offset)
    {
        Byte *_ptr = bytes.data() + offset;
        offset += sizeof(T);
        ptr = new (const_cast<std::remove_const_t<Byte> *>(_ptr)) T;
    }

    ~placement_ptr()
    {
        std::destroy_at(ptr);
    }

    placement_ptr() = delete;
    placement_ptr(placement_ptr const&) = delete;
    placement_ptr(placement_ptr&&) = delete;
    placement_ptr& operator=(placement_ptr const&) = delete;
    placement_ptr& operator=(placement_ptr&&) = delete;

    value_type *operator->() const noexcept
    {
        return ptr;
    }

    value_type& operator*() const noexcept
    {
        return *ptr;
    }
};

template<typename T, typename Byte>
placement_ptr<T, Byte> unsafe_make_placement_ptr(std::span<Byte> bytes, std::size_t& offset)
{
    return placement_ptr<T, Byte>(bytes, offset);
}

template<typename T, typename Byte>
placement_ptr<T, Byte> unsafe_make_placement_ptr(std::span<Byte> bytes, std::size_t&& offset = 0)
{
    std::size_t _offset = offset;
    return unsafe_make_placement_ptr<T>(bytes, _offset);
}

template<typename T, typename Byte>
bool check_placement_ptr(std::span<Byte> bytes, std::size_t offset = 0)
{
    return check_alignment<T>(bytes.data()) && (offset + sizeof(T) <= size(bytes));
}

template<typename T, typename Byte>
placement_ptr<T, Byte> make_placement_ptr(std::span<Byte> bytes, std::size_t& offset)
{
    hi_parse_check(check_placement_ptr<T>(bytes, offset), "Parsing beyond end of buffer");
    return placement_ptr<T, Byte>(bytes, offset);
}

template<typename T, typename Byte>
placement_ptr<T, Byte> make_placement_ptr(std::span<Byte> bytes, std::size_t&& offset = 0)
{
    std::size_t _offset = offset;
    return make_placement_ptr<T>(bytes, _offset);
}

template<typename T, typename Byte>
class placement_array {
    static_assert(
        std::is_same_v<std::remove_cv_t<Byte>, std::byte> || std::is_same_v<std::remove_cv_t<Byte>, char> ||
            std::is_same_v<std::remove_cv_t<Byte>, unsigned char> || std::is_same_v<std::remove_cv_t<Byte>, signed char>,
        "Byte must be a byte type");
    static_assert(std::is_trivially_constructible_v<T>);
    static_assert(!std::is_const_v<T> || std::is_trivially_destructible_v<T>);

    using value_type = copy_cv_t<T, Byte>;

    Byte *_begin;
    Byte *_end;

public:
    placement_array(std::span<Byte> bytes, std::size_t& offset, std::size_t n)
    {
        hilet bytes_ = bytes.data();

        _begin = bytes_ + offset;
        offset += sizeof(T) * n;
        _end = bytes_ + offset;

        for (auto i = 0_uz; i < n; i++) {
            [[maybe_unused]] auto *ptr = new (const_cast<std::remove_cv_t<Byte> *>(_begin + i * sizeof(T))) T;
        }
    }

    placement_array(placement_array const&) = delete;
    placement_array(placement_array&&) = delete;
    placement_array& operator=(placement_array const&) = delete;
    placement_array& operator=(placement_array&&) = delete;

    ~placement_array()
    {
        std::destroy(begin(), end());
    }

    std::size_t size() const noexcept
    {
        return std::distance(begin(), end());
    }

    bool contains(std::size_t index) const noexcept
    {
        return index < size();
    }

    value_type *begin() const noexcept
    {
        return std::launder(reinterpret_cast<value_type *>(_begin));
    }

    value_type *end() const noexcept
    {
        return std::launder(reinterpret_cast<value_type *>(_end));
    }

    value_type& operator[](ssize_t offset) const noexcept
    {
        return *(begin() + offset);
    }
};

template<typename T, typename Byte>
placement_array<T, Byte> unsafe_make_placement_array(std::span<Byte> bytes, std::size_t& offset, std::size_t n)
{
    return placement_array<T, Byte>(bytes, offset, n);
}

template<typename T, typename Byte>
placement_array<T, Byte> unsafe_make_placement_array(std::span<Byte> bytes, std::size_t&& offset, std::size_t n)
{
    std::size_t _offset = offset;
    return unsafe_make_placement_array<T>(bytes, _offset, n);
}

template<typename T, typename Byte>
placement_array<T, Byte> unsafe_make_placement_array(std::span<Byte> bytes, std::size_t& offset)
{
    hilet n = bytes.size() / sizeof(T);
    return unsafe_make_placement_array<T>(bytes, offset, n);
}

template<typename T, typename Byte>
placement_array<T, Byte> unsafe_make_placement_array(std::span<Byte> bytes, std::size_t&& offset = 0)
{
    std::size_t _offset = offset;
    return unsafe_make_placement_array<T>(bytes, _offset);
}

template<typename T, typename Byte>
bool check_placement_array(std::span<Byte> bytes, std::size_t offset, std::size_t n)
{
    return check_alignment<T>(bytes.data()) && (offset + (n * sizeof(T)) <= bytes.size());
}

template<typename T, typename Byte>
bool check_placement_array(std::span<Byte> bytes, std::size_t offset)
{
    return check_alignment<T>(bytes.data());
}

template<typename T, typename Byte>
placement_array<T, Byte> make_placement_array(std::span<Byte> bytes, std::size_t& offset, std::size_t n)
{
    hi_parse_check(check_placement_array<T>(bytes, offset, n), "Parsing beyond end of buffer");
    return placement_array<T, Byte>(bytes, offset, n);
}

template<typename T, typename Byte>
placement_array<T, Byte> make_placement_array(std::span<Byte> bytes, std::size_t&& offset, std::size_t n)
{
    std::size_t _offset = offset;
    return make_placement_array<T>(bytes, _offset, n);
}

template<typename T, typename Byte>
placement_array<T, Byte> make_placement_array(std::span<Byte> bytes, std::size_t& offset)
{
    hilet n = bytes.size() / ssizeof(T);
    return make_placement_array<T>(bytes, offset, n);
}

template<typename T, typename Byte>
placement_array<T, Byte> make_placement_array(std::span<Byte> bytes, std::size_t&& offset = 0)
{
    std::size_t _offset = offset;
    return make_placement_array<T>(bytes, _offset);
}

} // namespace hi::inline v1
