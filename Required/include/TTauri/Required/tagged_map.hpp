// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/string_tag.hpp"
#include <array>

namespace TTauri {

template<typename T, string_tag... Tags>
class tagged_map {
private:
    std::array<T, sizeof...(Tags)> data;

public:
    static constexpr size_t size() noexcept {
        return sizeof...(Tags);
    }

    static constexpr string_tag get_tag(size_t i) noexcept {
        return tag_at_index<Tags...>(i);
    }

    static constexpr bool has(string_tag tag) noexcept {
        return count_tag_if<Tags...>(tag);
    }

    constexpr auto begin() noexcept {
        return data.begin();
    }

    constexpr auto end() noexcept {
        return data.end();
    }

    constexpr auto begin() const noexcept {
        return data.begin();
    }

    constexpr auto end() const noexcept {
        return data.end();
    }

    
    constexpr T &operator[](size_t i) noexcept {
        return data[i];
    }

    constexpr T const &operator[](size_t i) const noexcept {
        return data[i];
    }

    constexpr T &get(string_tag tag) noexcept {
        return data[index_of_tag<Tags...>(tag)];
    }

    constexpr T const &get(string_tag tag) const noexcept {
        return data[index_of_tag<Tags...>(tag)];
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
