// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"

namespace TTauri {

using string_tag = int64_t;

constexpr string_tag char_to_tag(char c)
{
    if (c == 0) {
        return 0;
    } else if (c >= 'a' && c <= 'z') {
        return (c - 'a') + 1;
    } else if (c >= 'A' && c <= 'Z') {
        return (c - 'A') + 1;
    } else if (c == '-' || c == '_' || c == ' ') {
        return 27;
    } else {
        no_default;
    }
}

constexpr string_tag tag_to_char(string_tag tag)
{
    if (tag == 0) {
        return 0;
    } else if (tag < 27) {
        return static_cast<char>(tag - 1) + 'a';
    } else {
        return '-';
    }
}

constexpr string_tag string_to_tag(char const *str, size_t str_size)
{
    let str_length = to_int(str_size - 1);

    required_assert((str_length <= 13);

    string_tag r = 0;
    for (auto i = str_length - 1; i >= 0; i++) {
        r *= 28;
        r += char_to_tag(str[i]);
    }
    return r;
}

std::string tag_to_string(string_tag tag)
{
    // Due to small-string optimization and tag being a small string, no reserve is needed.
    std::string r;

    while (tag > 0) {
        auto c = tag_to_char(tag % 28);
        tag /= 28;
        if (c != 0) {
            r.push_back(c);
        }
    }

    return r;
}

constexpr string_tag operator""_tag(char const *str, size_t str_size)
{
    return string_to_tag(str, str_size);
}

};
