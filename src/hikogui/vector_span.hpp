// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <span>
#include <iterator>
#include <memory>
#include <new>

namespace hi::inline v1 {

template<typename T>
class vector_span_iterator {
public:
    using value_type = T;
    using difference_type = ssize_t;

private:
    value_type *ptr;

public:
    vector_span_iterator() noexcept : ptr(nullptr) {}
    vector_span_iterator(vector_span_iterator const &other) noexcept = default;
    vector_span_iterator(vector_span_iterator &&other) noexcept = default;
    vector_span_iterator &operator=(vector_span_iterator const &other) noexcept = default;
    vector_span_iterator &operator=(vector_span_iterator &&other) noexcept = default;
    ~vector_span_iterator() = default;

    vector_span_iterator(value_type *ptr) noexcept : ptr(ptr)
    {
        hi_assert_not_null(ptr);
    }

    vector_span_iterator &operator=(value_type *ptr) noexcept
    {
        hi_assert_not_null(ptr);
        this->ptr = ptr;
        return *this;
    }

    [[nodiscard]] value_type &operator*() noexcept
    {
        return *std::launder(ptr);
    }
    [[nodiscard]] value_type const &operator*() const noexcept
    {
        return *std::launder(ptr);
    }
    [[nodiscard]] value_type *operator->() noexcept
    {
        return std::launder(ptr);
    }
    [[nodiscard]] value_type const *operator->() const noexcept
    {
        return std::launder(ptr);
    }
    [[nodiscard]] value_type &operator[](std::size_t i) noexcept
    {
        return *std::launder(ptr + i);
    }
    [[nodiscard]] value_type const &operator[](std::size_t i) const noexcept
    {
        return *std::launder(ptr + i);
    }

    vector_span_iterator &operator++() noexcept
    {
        ++ptr;
        return *this;
    }
    vector_span_iterator operator++(int) noexcept
    {
        auto tmp = *this;
        ++ptr;
        return tmp;
    }
    vector_span_iterator &operator--() noexcept
    {
        --ptr;
        return *this;
    }
    vector_span_iterator operator--(int) noexcept
    {
        auto tmp = *this;
        --ptr;
        return tmp;
    }

    vector_span_iterator &operator+=(ssize_t rhs) noexcept
    {
        ptr += rhs;
        return *this;
    }
    vector_span_iterator &operator-=(ssize_t rhs) noexcept
    {
        ptr -= rhs;
        return *this;
    }

    [[nodiscard]] friend bool operator==(vector_span_iterator const &lhs, vector_span_iterator const &rhs) noexcept
    {
        return lhs.ptr == rhs.ptr;
    }

    [[nodiscard]] friend auto operator<=>(vector_span_iterator const &lhs, vector_span_iterator const &rhs) noexcept
    {
        return lhs.ptr <=> rhs.ptr;
    }

    [[nodiscard]] friend vector_span_iterator operator+(vector_span_iterator const &lhs, ssize_t rhs) noexcept
    {
        return vector_span_iterator{lhs.ptr + rhs};
    }
    [[nodiscard]] friend vector_span_iterator operator-(vector_span_iterator const &lhs, ssize_t rhs) noexcept
    {
        return vector_span_iterator{lhs.ptr - rhs};
    }
    [[nodiscard]] friend vector_span_iterator operator+(ssize_t lhs, vector_span_iterator const &rhs) noexcept
    {
        return vector_span_iterator{lhs + rhs.ptr};
    }

    [[nodiscard]] friend difference_type operator-(vector_span_iterator const &lhs, vector_span_iterator const &rhs) noexcept
    {
        return lhs.ptr - rhs.ptr;
    }
};

template<typename T>
class vector_span {
public:
    using value_type = T;
    using iterator = vector_span_iterator<value_type>;
    using const_iterator = vector_span_iterator<value_type const>;

private:
    value_type *_begin;
    value_type *_end;
    value_type *_max;

public:
    constexpr vector_span() noexcept : _begin(nullptr), _end(nullptr), _max(nullptr) {}

    constexpr vector_span(value_type *buffer, ssize_t nr_elements) noexcept :
        _begin(buffer), _end(buffer), _max(buffer + nr_elements)
    {
        hi_axiom(nr_elements >= 0);
    }

    constexpr vector_span(std::span<value_type> span) noexcept :
        _begin(span.data()), _end(span.data()), _max(span.data() + span.size())
    {
    }

    constexpr vector_span(vector_span const& other) = default;
    constexpr vector_span(vector_span&& other) = default;
    constexpr vector_span& operator=(vector_span const& other) = default;
    constexpr vector_span& operator=(vector_span&& other) = default;
    ~vector_span() = default;

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return _begin;
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return _begin;
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return _begin;
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return _end;
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return _end;
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return _end;
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return std::distance(_begin, _end);
    }

    [[nodiscard]] constexpr value_type& operator[](std::size_t i) noexcept
    {
        hi_axiom_bounds(i, *this);
        return _begin[i];
    }
    [[nodiscard]] constexpr value_type const& operator[](std::size_t i) const noexcept
    {
        hi_axiom_bounds(i, *this);
        return _begin[i];
    }

    constexpr value_type& front() noexcept
    {
        hi_axiom(_end != _begin);
        return *_begin;
    }

    constexpr value_type const& front() const noexcept
    {
        hi_axiom(_end != _begin);
        return *_begin;
    }

    constexpr value_type& back() noexcept
    {
        hi_axiom(_end != _begin);
        return *(_end - 1);
    }

    constexpr value_type const& back() const noexcept
    {
        hi_axiom(_end != _begin);
        return *(_end - 1);
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _begin == _end;
    }

    [[nodiscard]] constexpr bool full() const noexcept
    {
        return _end == _max;
    }

    constexpr void clear() noexcept
    {
        for (auto ptr = _begin; ptr != _end; ++ptr) {
            std::destroy_at(ptr);
        }
        _end = _begin;
    }

    constexpr void push_back(value_type const& rhs) noexcept
    {
        hi_axiom(_end != _max);
        std::construct_at(_end++, rhs);
    }

    constexpr void push_back(value_type&& rhs) noexcept
    {
        hi_axiom(_end != _max);
        std::construct_at(_end++, std::move(rhs));
    }

    template<typename... Args>
    constexpr void emplace_back(Args&&...args) noexcept
    {
        hi_axiom(_end != _max);
        std::construct_at(_end++, std::forward<Args>(args)...);
    }

    constexpr void pop_back() noexcept
    {
        hi_axiom(_end != _begin);
        std::destroy_at(--_end);
    }
};

} // namespace hi::inline v1
