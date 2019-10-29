// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <array>

namespace TTauri {

template<typename T,size_t N>
class small_vector {
    using value_type = T;
    using array_type = std::array<T,N>;
    using iterator = typename array_type::iterator;
    using const_iterator = typename array_type::iterator;

    array_type items;
    iterator _end;

public:
    constexpr small_vector() noexcept {
        _end = items.begin();
    }

    constexpr auto begin() noexcept {
        return items.begin();
    }

    constexpr auto end() noexcept {
        return _end;
    }

    constexpr size_t size() const noexcept {
        return static_cast<size_t>(_end - items.begin());
    }

    constexpr void clear() noexcept {
        _end = items.begin();
    }

    constexpr bool push_back(value_type &&value) noexcept {
        if (_end == items.end()) {
            return false;
        }
        *(_end++) = std::move(value);
        return true;
    }

    constexpr bool push_back(value_type const &value) noexcept {
        if (_end == items.end()) {
            return false;
        }
        *(_end++) = value;
        return true;
    }

};

}