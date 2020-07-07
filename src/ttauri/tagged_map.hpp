// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/string_tag.hpp"
#include <typeinfo>
#include <typeindex>
#include <array>

namespace tt {

template<typename T, typename... Tags>
class tagged_map {
private:
    std::array<T, sizeof...(Tags)> data;

public:
    static constexpr size_t size() noexcept {
        return sizeof...(Tags);
    }

    static std::type_index get_tag(size_t i) noexcept {
        return tag_at_index<Tags...>(i);
    }

    static bool has(std::type_index tag) noexcept {
        return has_tag<Tags...>(tag);
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

    T &get(std::type_index tag) noexcept {
        return data[index_of_tag<Tags...>(tag)];
    }

    T const &get(std::type_index tag) const noexcept {
        return data[index_of_tag<Tags...>(tag)];
    }

    template<typename Tag>
    constexpr T &get() noexcept {
        return data[index_of_tag<Tag,Tags...>()];
    }

    template<typename Tag>
    constexpr T const &get() const noexcept {
        return data[index_of_tag<Tag,Tags...>()];
    }
};

}
