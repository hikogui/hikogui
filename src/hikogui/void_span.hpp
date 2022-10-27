// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "assert.hpp"
#include "memory.hpp"
#include "byte_string.hpp"
#include <span>
#include <cstddef>
#include <bit>
#include <type_traits>
#include <string_view>

namespace hi::inline v1 {

class void_span {
public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = void *;
    using const_pointer = void const *;

    static constexpr std::size_t extent = std::dynamic_extent;

    constexpr ~void_span() = default;
    constexpr void_span() noexcept = default;
    constexpr void_span(void_span const&) noexcept = default;
    constexpr void_span(void_span&&) noexcept = default;
    constexpr void_span& operator=(void_span const&) noexcept = default;
    constexpr void_span& operator=(void_span&&) noexcept = default;
    constexpr void_span(void *pointer, size_t size) noexcept : _pointer(pointer), _size(size)
    {
        hi_assert(pointer != nullptr or size == 0);
    }

    template<typename T, std::size_t N>
    constexpr void_span(std::array<T, N>& rhs) noexcept requires(not std::is_const_v<T>) : void_span(rhs.data(), rhs.size())
    {
    }

    template<typename T, std::size_t N>
    constexpr void_span(std::span<T, N>& rhs) noexcept requires(not std::is_const_v<T>) : void_span(rhs.data(), rhs.size())
    {
    }

    [[nodiscard]] constexpr pointer data() const noexcept
    {
        return _pointer;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return _size;
    }

    [[nodiscard]] constexpr size_type size_bytes() const noexcept
    {
        return _size;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _size == 0;
    }

    template<std::size_t N>
    [[nodiscard]] constexpr void_span first() const noexcept
    {
        hi_axiom(N <= _size);
        return {_pointer, N};
    }

    [[nodiscard]] constexpr void_span first(size_type n) const noexcept
    {
        hi_axiom(n <= _size);
        return {_pointer, n};
    }

    template<std::size_t N>
    [[nodiscard]] constexpr void_span last() const noexcept
    {
        hi_axiom(N <= _size);
        return {advance_bytes(_pointer, _size - N), N};
    }

    [[nodiscard]] constexpr void_span last(size_type n) const noexcept
    {
        hi_axiom(n <= _size);
        return {advance_bytes(_pointer, _size - n), n};
    }

    template<std::size_t Offset, std::size_t Count = std::dynamic_extent>
    [[nodiscard]] constexpr void_span subspan() const noexcept
    {
        if constexpr (Count == std::dynamic_extent) {
            hi_axiom(Offset <= _size);
            return {advance_bytes(_pointer, Offset), _size - Offset};
        } else {
            hi_axiom((Offset + Count) <= _size);
            return {advance_bytes(_pointer, Offset), Count};
        }
    }

    [[nodiscard]] constexpr void_span subspan(size_type offset, size_type count = std::dynamic_extent) const noexcept
    {
        hi_axiom(offset <= _size);
        if (count == std::dynamic_extent) {
            count = _size - offset;
        }

        hi_axiom(offset + count <= _size);
        return {advance_bytes(_pointer, _size - count), count};
    }

    template<typename T, size_t E = std::dynamic_extent>
    [[nodiscard]] constexpr friend std::span<T, E> as_span(void_span const& rhs) noexcept
    {
        if constexpr (E == std::dynamic_extent) {
            hi_axiom(std::bit_cast<std::uintptr_t>(rhs._pointer) % std::alignment_of_v<T> == 0);
            hi_axiom((rhs._size / sizeof(T)) * sizeof(T) == rhs._size);
            return {static_cast<T *>(rhs._pointer), rhs._size / sizeof(T)};

        } else {
            hi_axiom(std::bit_cast<std::uintptr_t>(rhs._pointer) % std::alignment_of_v<T> == 0);
            hi_axiom(E * sizeof(T) <= rhs._size);
            return {static_cast<T *>(rhs._pointer), E};
        }
    }

    template<typename CharT, typename Traits = std::char_traits<CharT>>
    [[nodiscard]] constexpr friend std::basic_string_view<CharT, Traits> as_basic_string_view(void_span const& rhs) noexcept
    {
        hi_axiom(std::bit_cast<std::uintptr_t>(rhs._pointer) % std::alignment_of_v<CharT> == 0);
        hi_axiom((rhs._size / sizeof(CharT)) * sizeof(CharT) == rhs._size);
        return {static_cast<CharT const *>(rhs._pointer), rhs._size / sizeof(CharT)};
    }

    [[nodiscard]] constexpr friend std::string_view as_string_view(void_span const& rhs) noexcept
    {
        return as_basic_string_view<char>(rhs);
    }

    [[nodiscard]] constexpr friend bstring_view as_bstring_view(void_span const& rhs) noexcept
    {
        return as_basic_string_view<std::byte, byte_char_traits>(rhs);
    }

private:
    pointer _pointer = nullptr;
    size_type _size = 0;
};

class const_void_span {
public:
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = void const *;
    using const_pointer = void const *;

    static constexpr std::size_t extent = std::dynamic_extent;

    constexpr ~const_void_span() = default;
    constexpr const_void_span() noexcept = default;
    constexpr const_void_span(const_void_span const&) noexcept = default;
    constexpr const_void_span(const_void_span&&) noexcept = default;
    constexpr const_void_span& operator=(const_void_span const&) noexcept = default;
    constexpr const_void_span& operator=(const_void_span&&) noexcept = default;
    constexpr const_void_span(void const *pointer, size_t size) noexcept : _pointer(pointer), _size(size)
    {
        hi_assert(pointer != nullptr or size == 0);
    }

    constexpr const_void_span(void_span const& rhs) noexcept : const_void_span(rhs.data(), rhs.size()) {}

    template<typename T, std::size_t N>
    constexpr const_void_span(std::array<T, N> const& rhs) noexcept : const_void_span(rhs.data(), rhs.size())
    {
    }

    template<typename T, std::size_t N>
    constexpr const_void_span(std::span<T, N> const& rhs) noexcept : const_void_span(rhs.data(), rhs.size())
    {
    }

    [[nodiscard]] constexpr pointer data() const noexcept
    {
        return _pointer;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return _size;
    }

    [[nodiscard]] constexpr size_type size_bytes() const noexcept
    {
        return _size;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _size == 0;
    }

    template<std::size_t N>
    [[nodiscard]] constexpr const_void_span first() const noexcept
    {
        hi_axiom(N <= _size);
        return {_pointer, N};
    }

    [[nodiscard]] constexpr const_void_span first(size_type n) const noexcept
    {
        hi_axiom(n <= _size);
        return {_pointer, n};
    }

    template<std::size_t N>
    [[nodiscard]] constexpr const_void_span last() const noexcept
    {
        hi_axiom(N <= _size);
        return {advance_bytes(_pointer, _size - N), N};
    }

    [[nodiscard]] constexpr const_void_span last(size_type n) const noexcept
    {
        hi_axiom(n <= _size);
        return {advance_bytes(_pointer, _size - n), n};
    }

    template<std::size_t Offset, std::size_t Count = std::dynamic_extent>
    [[nodiscard]] constexpr const_void_span subspan() const noexcept
    {
        if constexpr (Count == std::dynamic_extent) {
            hi_axiom(Offset <= _size);
            return {advance_bytes(_pointer, Offset), _size - Offset};
        } else {
            hi_axiom((Offset + Count) <= _size);
            return {advance_bytes(_pointer, Offset), Count};
        }
    }

    [[nodiscard]] constexpr const_void_span subspan(size_type offset, size_type count = std::dynamic_extent) const noexcept
    {
        hi_axiom(offset <= _size);
        if (count == std::dynamic_extent) {
            count = _size - offset;
        }

        hi_axiom(offset + count <= _size);
        return {advance_bytes(_pointer, _size - count), count};
    }

    template<typename T, size_t E = std::dynamic_extent>
    [[nodiscard]] constexpr friend std::span<T, E> as_span(const_void_span const& rhs) noexcept requires(std::is_const_v<T>)
    {
        if constexpr (E == std::dynamic_extent) {
            hi_axiom(std::bit_cast<std::uintptr_t>(rhs._pointer) % std::alignment_of_v<T> == 0);
            hi_axiom((rhs._size / sizeof(T)) * sizeof(T) == rhs._size);
            return {static_cast<T *>(rhs._pointer), rhs._size / sizeof(T)};

        } else {
            hi_axiom(std::bit_cast<std::uintptr_t>(rhs._pointer) % std::alignment_of_v<T> == 0);
            hi_axiom(E * sizeof(T) <= rhs._size);
            return {static_cast<T *>(rhs._pointer), E};
        }
    }

    template<typename CharT, typename Traits = std::char_traits<CharT>>
    [[nodiscard]] constexpr friend std::basic_string_view<CharT, Traits> as_basic_string_view(const_void_span const& rhs) noexcept
    {
        hi_axiom(std::bit_cast<std::uintptr_t>(rhs._pointer) % std::alignment_of_v<CharT> == 0);
        hi_axiom((rhs._size / sizeof(CharT)) * sizeof(CharT) == rhs._size);
        return {static_cast<CharT const *>(rhs._pointer), rhs._size / sizeof(CharT)};
    }

    [[nodiscard]] constexpr friend std::string_view as_string_view(const_void_span const& rhs) noexcept
    {
        return as_basic_string_view<char>(rhs);
    }

    [[nodiscard]] constexpr friend bstring_view as_bstring_view(const_void_span const& rhs) noexcept
    {
        return as_basic_string_view<std::byte, byte_char_traits>(rhs);
    }

private:
    pointer _pointer = nullptr;
    size_type _size = 0;
};

} // namespace hi::inline v1
