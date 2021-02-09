// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "tag.hpp"
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
