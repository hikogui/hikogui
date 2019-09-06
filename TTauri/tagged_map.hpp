// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "string_tag.hpp"
#include <array>

namespace TTauri {

template<typename T, string_tag... Tags>
class tagged_map {
private:
    std::array<T, sizeof...(Tags)> data;

public:
    constexpr size_t size() const noexcept {
        return data.size();
    }

    constexpr string_tag get_tag(size_t i) const noexcept {
        return tag_at_index<Tags...>(i);
    }

    constexpr T &operator[](size_t i) noexcept {
        return data[i];
    }

    constexpr T const &operator[](size_t i) const noexcept {
        return data[i];
    }

    template<string_tag Tag>
    constexpr T &get() noexcept {
        return data[index_of_tag<Tags...>(Tag)];
    }

    template<string_tag Tag>
    constexpr T const &get() const noexcept {
        return data[index_of_tag<Tags...>(Tag)];
    }
};

}