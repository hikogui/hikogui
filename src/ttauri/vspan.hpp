// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "assert.hpp"
#include <span>
#include <iterator>
#include <memory>
#include <new>

namespace tt {

template<typename T>
class vspan_iterator {
public:
    using value_type = T;
    using difference_type = ssize_t;

private:
    value_type *ptr;

public:
    vspan_iterator() noexcept : ptr(nullptr) {}
    vspan_iterator(vspan_iterator const &other) noexcept = default;
    vspan_iterator(vspan_iterator &&other) noexcept = default;
    vspan_iterator &operator=(vspan_iterator const &other) noexcept = default;
    vspan_iterator &operator=(vspan_iterator &&other) noexcept = default;
    ~vspan_iterator() = default;

    vspan_iterator(value_type *ptr) noexcept : ptr(ptr) {
        tt_assume(ptr != nullptr);
    }

    vspan_iterator &operator=(value_type *ptr) noexcept {
        tt_assume(ptr != nullptr);
        this->ptr = ptr;
        return *this;
    }

    [[nodiscard]] value_type &operator*() noexcept { return *std::launder(ptr); }
    [[nodiscard]] value_type const &operator*() const noexcept { return *std::launder(ptr); }
    [[nodiscard]] value_type *operator->() noexcept { return std::launder(ptr); }
    [[nodiscard]] value_type const *operator->() const noexcept { return std::launder(ptr); }
    [[nodiscard]] value_type &operator[](size_t i) noexcept { return *std::launder(ptr + i); }
    [[nodiscard]] value_type const &operator[](size_t i) const noexcept { return *std::launder(ptr + i); }

    vspan_iterator &operator++() noexcept { ++ptr; return *this; }
    vspan_iterator operator++(int) noexcept { auto tmp = *this; ++ptr; return tmp; }
    vspan_iterator &operator--() noexcept { --ptr; return *this; }
    vspan_iterator operator--(int) noexcept { auto tmp = *this; --ptr; return tmp; }

    vspan_iterator &operator+=(ssize_t rhs) noexcept { ptr += rhs; return *this; }
    vspan_iterator &operator-=(ssize_t rhs) noexcept { ptr -= rhs; return *this; }

    [[nodiscard]] friend bool operator==(vspan_iterator const &lhs, vspan_iterator const &rhs) noexcept { return lhs.ptr == rhs.ptr; }
    [[nodiscard]] friend bool operator!=(vspan_iterator const &lhs, vspan_iterator const &rhs) noexcept { return lhs.ptr != rhs.ptr; }
    [[nodiscard]] friend bool operator<(vspan_iterator const &lhs, vspan_iterator const &rhs) noexcept { return lhs.ptr < rhs.ptr; }
    [[nodiscard]] friend bool operator>(vspan_iterator const &lhs, vspan_iterator const &rhs) noexcept { return lhs.ptr > rhs.ptr; }
    [[nodiscard]] friend bool operator<=(vspan_iterator const &lhs, vspan_iterator const &rhs) noexcept { return lhs.ptr <= rhs.ptr; }
    [[nodiscard]] friend bool operator>=(vspan_iterator const &lhs, vspan_iterator const &rhs) noexcept { return lhs.ptr >= rhs.ptr; }

    [[nodiscard]] friend vspan_iterator operator+(vspan_iterator const &lhs, ssize_t rhs) noexcept { return vspan_iterator{lhs.ptr + rhs}; }
    [[nodiscard]] friend vspan_iterator operator-(vspan_iterator const &lhs, ssize_t rhs) noexcept { return vspan_iterator{lhs.ptr - rhs}; }
    [[nodiscard]] friend vspan_iterator operator+(ssize_t lhs, vspan_iterator const &rhs) noexcept { return vspan_iterator{lhs + rhs.ptr}; }

    [[nodiscard]] friend difference_type operator-(vspan_iterator const &lhs, vspan_iterator const &rhs) noexcept { return lhs.ptr - rhs.ptr; }
};

template<typename T>
class vspan {
public:
    using value_type = T;
    using iterator = vspan_iterator<value_type>;
    using const_iterator = vspan_iterator<value_type const>;

private:
    value_type *_begin;
    value_type *_end;
    value_type *_max;

public:
    vspan() noexcept :
        _begin(nullptr), _end(nullptr), _max(nullptr) {}

    vspan(value_type *buffer, ssize_t nr_elements) noexcept :
        _begin(buffer), _end(buffer), _max(buffer + nr_elements)
    {
        tt_assume(nr_elements >= 0);
    }

    vspan(std::span<value_type> span) noexcept :
        _begin(span.data()), _end(span.data()), _max(span.data() + span.size()) {}

    vspan(vspan const &other) = default;
    vspan(vspan &&other) = default;
    vspan &operator=(vspan const &other) = default;
    vspan &operator=(vspan &&other) = default;
    ~vspan() = default;

    [[nodiscard]] iterator begin() noexcept { return _begin; }
    [[nodiscard]] const_iterator begin() const noexcept { return _begin; }
    [[nodiscard]] const_iterator cbegin() const noexcept { return _begin; }

    [[nodiscard]] iterator end() noexcept { return _end; }
    [[nodiscard]] const_iterator end() const noexcept { return _end; }
    [[nodiscard]] const_iterator cend() const noexcept { return _end; }

    [[nodiscard]] size_t size() const noexcept { return std::distance(_begin, _end); }

    [[nodiscard]] value_type &operator[](size_t i) noexcept { tt_assume(i < size()); return *std::launder(_begin + i); }
    [[nodiscard]] value_type const &operator[](size_t i) const noexcept { tt_assume(i < size()); return *std::launder(_begin + i); }

    value_type &front() noexcept { tt_assume(_end != _begin); return *std::launder(_begin); }
    value_type const &front() const noexcept { tt_assume(_end != _begin); return *std::launder(_begin); }
    value_type &back() noexcept { tt_assume(_end != _begin); return *std::launder(_end - 1); }
    value_type const &back() const noexcept { tt_assume(_end != _begin); return *std::launder(_end - 1); }

    vspan &clear() noexcept {
        for (auto i = _begin; i != _end; ++i) {
            std::destroy_at(std::launder(i));
        }
        _end = _begin;
        return *this;
    }

    void push_back(value_type const &rhs) noexcept {
        tt_assume(_end != _max);
        // Since we throw away the pointer, we have to std::launder all access to this object.
        [[maybe_unused]] value_type *ptr = new (_end) value_type(rhs);
        ++_end;
    }

    void push_back(value_type &&rhs) noexcept {
        tt_assume(_end != _max);
        // Since we throw away the pointer, we have to std::launder all access to this object.
        [[maybe_unused]] value_type *ptr = new (_end) value_type(std::move(rhs));
        ++_end;
    }

    template<typename... Args>
    void emplace_back(Args &&... args) noexcept {
        tt_assume(_end != _max);
        // Since we throw away the pointer, we have to std::launder all access to this object.
        [[maybe_unused]] value_type *ptr = new (_end) value_type(std::forward<Args>(args)...);
        ++_end;
    }

    void pop_back() noexcept {
        tt_assume(_end != _begin);
        --_end;
        std::destroy_at(std::launder(_end));
    }
};

}
