// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <numeric>


namespace TTauri {

struct URLUserinfo {
    std::string username = {};
    std::optional<std::string> password = {};

    URLUserinfo() = default;
    URLUserinfo(std::string const &userinfo);
};

std::string to_string(URLUserinfo const &userinfo);

bool operator==(URLUserinfo const &lhs, URLUserinfo const &rhs);
bool operator<(URLUserinfo const &lhs, URLUserinfo const &rhs);

}

namespace std {

template<>
struct hash<TTauri::URLUserinfo> {
    typedef TTauri::URLUserinfo argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& userinfo) const noexcept {
        return
            std::hash<decltype(userinfo.username)>{}(userinfo.username) ^
            std::hash<decltype(userinfo.password)>{}(userinfo.password);
    }
};

}
