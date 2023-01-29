// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <span>

hi_warning_push();
// C26492: Don't use const_cast to cast away const or volatile (type.3).
// placement new requires non-const pointer, even if the non-constructor initializer doesn't
// modify the memory.
hi_warning_ignore_msvc(26492);
// We are going to remove placement.hpp anyway.
hi_warning_ignore_msvc(26403);
hi_warning_ignore_msvc(26460);

namespace hi::inline v1 {
template<typename T>
inline bool check_alignment(void const *ptr) noexcept
{
    return std::bit_cast<uintptr_t>(ptr) % alignof(T) == 0;
}

template<typename T>
class placement_ptr {
public:
    static_assert(std::is_trivially_default_constructible_v<T>);
    static_assert(std::is_trivially_destructible_v<T>);

    using value_type = T;

    template<byte_like Byte>
    placement_ptr(std::span<Byte> bytes, std::size_t& offset)
    {
        if (offset + sizeof(value_type) > bytes.size()) {
            throw std::bad_alloc();
        }

        auto ptr = bytes.data() + offset;
        offset += sizeof(value_type);
        std::uninitialized_default_construct_n(reinterpret_cast<value_type *>(ptr), 1);
        _ptr = std::launder(reinterpret_cast<value_type *>(ptr));
    }

    ~placement_ptr()
    {
        std::destroy_at(_ptr);
    }

    placement_ptr() = delete;
    placement_ptr(placement_ptr const&) = delete;
    placement_ptr(placement_ptr&&) = delete;
    placement_ptr& operator=(placement_ptr const&) = delete;
    placement_ptr& operator=(placement_ptr&&) = delete;

    value_type *operator->() const noexcept
    {
        return _ptr;
    }

    value_type& operator*() const noexcept
    {
        return *_ptr;
    }

private:
    value_type *_ptr;
};

template<typename T, typename Byte>
[[nodiscard]] auto make_placement_ptr(std::span<Byte> bytes, std::size_t& offset)
{
    return placement_ptr<copy_cv_t<T, Byte>>(bytes, offset);
}

template<typename T, typename Byte>
[[nodiscard]] auto make_placement_ptr(std::span<Byte> bytes, std::size_t&& offset = 0)
{
    std::size_t _offset = offset;
    return make_placement_ptr<T>(bytes, _offset);
}

template<typename T>
class placement_array {
public:
    static_assert(std::is_trivially_default_constructible_v<T>);
    static_assert(std::is_trivially_destructible_v<T>);

    using value_type = T;
    using container_type = std::span<value_type>;
    using iterator = container_type::iterator;
    using reference = container_type::reference;
    using pointer = container_type::pointer;

    template<byte_like Byte>
    placement_array(std::span<Byte> bytes, std::size_t& offset, std::size_t n)
    {
        if (offset + n * sizeof(T) > bytes.size()) {
            throw std::bad_alloc();
        }

        hilet ptr = bytes.data() + offset;
        offset += sizeof(T) * n;

        std::uninitialized_default_construct_n(reinterpret_cast<value_type *>(ptr), n);
        _items = {std::launder(reinterpret_cast<value_type *>(ptr)), n};
    }

    placement_array(placement_array const&) = delete;
    placement_array(placement_array&&) = delete;
    placement_array& operator=(placement_array const&) = delete;
    placement_array& operator=(placement_array&&) = delete;

    ~placement_array()
    {
        std::destroy(begin(), end());
    }

    operator std::span<value_type>() const noexcept
    {
        return _items;
    }

    std::size_t size() const noexcept
    {
        return std::distance(begin(), end());
    }

    iterator begin() const noexcept
    {
        return _items.begin();
    }

    iterator end() const noexcept
    {
        return _items.end();
    }

    reference operator[](ssize_t offset) const noexcept
    {
        return _items[offset];
    }

private:
    std::span<value_type> _items;
};

template<typename T, byte_like Byte>
[[nodiscard]] auto make_placement_array(std::span<Byte> bytes, std::size_t& offset, std::size_t n)
{
    return placement_array<copy_cv_t<T, Byte>>(bytes, offset, n);
}

template<typename T, byte_like Byte>
[[nodiscard]] auto make_placement_array(std::span<Byte> bytes, std::size_t&& offset, std::size_t n)
{
    std::size_t _offset = offset;
    return make_placement_array<T>(bytes, _offset, n);
}

template<typename T, byte_like Byte>
[[nodiscard]] auto make_placement_array(std::span<Byte> bytes, std::size_t& offset)
{
    hilet n = bytes.size() / ssizeof(T);
    return make_placement_array<T>(bytes, offset, n);
}

template<typename T, byte_like Byte>
[[nodiscard]] auto make_placement_array(std::span<Byte> bytes, std::size_t&& offset = 0)
{
    std::size_t _offset = offset;
    return make_placement_array<T>(bytes, _offset);
}

} // namespace hi::inline v1
