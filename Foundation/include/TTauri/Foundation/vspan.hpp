// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/assert.hpp"
#include <gsl/gsl>
#include <iterator>
#include <memory>
#include <new>

namespace TTauri {

template<typename T>
class vspan_endterator {
public:
    using value_type = T;
    using difference_type = ssize_t;

private:
    value_type *ptr;

public:
    vspan_endterator() noexcept : ptr(nullptr) {}
    vspan_endterator(vspan_endterator const &other) noexcept = default;
    vspan_endterator(vspan_endterator &&other) noexcept = default;
    vspan_endterator &operator=(vspan_endterator const &other) noexcept = default;
    vspan_endterator &operator=(vspan_endterator &&other) noexcept = default;
    ~vspan_endterator() = default;

    vspan_endterator(value_type *ptr) noexcept : ptr(ptr) {
        ttauri_assume(ptr != nullptr);
    }

    vspan_endterator &operator=(value_type *ptr) noexcept {
        ttauri_assume(ptr != nullptr);
        this->ptr = ptr;
        return *this;
    }

    [[nodiscard]] value_type &operator*() noexcept { return *std::launder(ptr); }
    [[nodiscard]] value_type const &operator*() const noexcept { return *std::launder(ptr); }
    [[nodiscard]] value_type *operator->() noexcept { return std::launder(ptr); }
    [[nodiscard]] value_type const *operator->() const noexcept { return std::launder(ptr); }
    [[nodiscard]] value_type &operator[](size_t i) noexcept { return *std::launder(ptr + i); }
    [[nodiscard]] value_type const &operator[](size_t i) const noexcept { return *std::launder(ptr + i); }

    vspan_endterator &operator++() noexcept { ++ptr; return *this; }
    vspan_endterator operator++(int) noexcept { auto tmp = *this; ++ptr; return tmp; }
    vspan_endterator &operator--() noexcept { --ptr; return *this; }
    vspan_endterator operator--(int) noexcept { auto tmp = *this; --ptr; return tmp; }

    vspan_endterator &operator+=(ssize_t rhs) noexcept { ptr += rhs; return *this; }
    vspan_endterator &operator-=(ssize_t rhs) noexcept { ptr -= rhs; return *this; }

    [[nodiscard]] friend bool operator==(vspan_endterator const &lhs, vspan_endterator const &rhs) noexcept { return lhs.ptr == rhs.ptr; }
    [[nodiscard]] friend bool operator!=(vspan_endterator const &lhs, vspan_endterator const &rhs) noexcept { return lhs.ptr != rhs.ptr; }
    [[nodiscard]] friend bool operator<(vspan_endterator const &lhs, vspan_endterator const &rhs) noexcept { return lhs.ptr < rhs.ptr; }
    [[nodiscard]] friend bool operator>(vspan_endterator const &lhs, vspan_endterator const &rhs) noexcept { return lhs.ptr > rhs.ptr; }
    [[nodiscard]] friend bool operator<=(vspan_endterator const &lhs, vspan_endterator const &rhs) noexcept { return lhs.ptr <= rhs.ptr; }
    [[nodiscard]] friend bool operator>=(vspan_endterator const &lhs, vspan_endterator const &rhs) noexcept { return lhs.ptr >= rhs.ptr; }

    [[nodiscard]] friend vspan_endterator operator+(vspan_endterator const &lhs, ssize_t rhs) noexcept { return vspan_endterator{lhs.ptr + rhs}; }
    [[nodiscard]] friend vspan_endterator operator-(vspan_endterator const &lhs, ssize_t rhs) noexcept { return vspan_endterator{lhs.ptr - rhs}; }
    [[nodiscard]] friend vspan_endterator operator+(ssize_t lhs, vspan_endterator const &rhs) noexcept { return vspan_endterator{lhs + rhs.ptr}; }

    [[nodiscard]] friend difference_type operator-(vspan_endterator const &lhs, vspan_endterator const &rhs) noexcept { return lhs.ptr - rhs.ptr; }
};

template<typename T>
class vspan {
public:
    using value_type = T;
    using iterator = vspan_endterator<value_type>;
    using const_endterator = vspan_endterator<value_type const>;

private:
    value_type *_begin;
    value_type *_end;
    value_type *_max;

public:
    vspan() noexcept :
        _begin(nullptr), _end(nullptr), _max(nullptr) {}

    vspan(value_type *buffer, ssize_t nr_elements) noexcept :
        _begin(buffer), _end(buffer), _max(buffer + nr_elments)
    {
        ttauri_assume(nr_elements >= 0);
    }

    vspan(gsl::span<value_type> span) noexcept :
        _begin(span.data()), _end(span.data()), _max(span.data() + span.size()) {}

    vspan(vspan const &other) = default;
    vspan(vspan &&other) = default;
    vspan &operator=(vspan const &other) = default;
    vspan &operator=(vspan &&other) = default;
    ~vspan() = default;

    [[nodiscard]] iterator begin() noexcept { return _begin; }
    [[nodiscard]] const_endterator begin() const noexcept { return _begin; }
    [[nodiscard]] const_endterator cbegin() const noexcept { return _begin; }

    [[nodiscard]] iterator end() noexcept { return _end; }
    [[nodiscard]] const_endterator end() const noexcept { return _end; }
    [[nodiscard]] const_endterator cend() const noexcept { return _end; }

    [[nodiscard]] size_t size() const noexcept { return std::distance(_begin, _end); }

    [[nodiscard]] value_type &operator[](size_t i) noexcept { ttauri_assume(i < size()); return *std::launder(_begin + i); }
    [[nodiscard]] value_type const &operator[](size_t i) const noexcept { ttauri_assume(i < size()); return *std::launder(_begin + i); }

    value_type &front() noexcept { ttauri_assume(_end != _begin); return *std::launder(_begin); }
    value_type const &front() const noexcept { ttauri_assume(_end != _begin); return *std::launder(_begin); }
    value_type &back() noexcept { ttauri_assume(_end != _begin); return *std::launder(_end - 1); }
    value_type const &back() const noexcept { ttauri_assume(_end != _begin); return *std::launder(_end - 1); }

    vspan &clear() noexcept {
        for (auto i = _begin; i != _end; ++i) {
            std::destroy_at(std::launder(i));
        }
        _end = _begin;
        return *this;
    }

    void push_back(value_type const &rhs) noexcept {
        ttauri_assume(_end != _max);
        // Since we throw away the pointer, we have to std::launder all access to this object.
        [[maybe_unused]] value_type *ptr = new (_end) value_type(rhs);
        ++_end;
    }

    void push_back(value_type &&rhs) noexcept {
        ttauri_assume(_end != _max);
        // Since we throw away the pointer, we have to std::launder all access to this object.
        [[maybe_unused]] value_type *ptr = new (_end) value_type(std::move(rhs));
        ++_end;
    }

    template<typename... Args>
    void emplace_back(Args &&... args) noexcept {
        ttauri_assume(_end != _max);
        // Since we throw away the pointer, we have to std::launder all access to this object.
        [[maybe_unused]] value_type *ptr = new (_end) value_type(std::forward<Args>(args)...);
        ++_end;
    }

    void pop_back() noexcept {
        ttauri_assume(_end != _begin);
        --_end;
        std::destroy_at(std::launder(_end));
    }
};

}