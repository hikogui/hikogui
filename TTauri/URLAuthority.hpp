// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "URLUserinfo.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <numeric>

namespace TTauri {

struct URLAuthority {
    std::optional<URLUserinfo> userinfo = {};
    std::string host = {};
    std::optional<std::string> port = {};

    URLAuthority() = default;
    URLAuthority(std::string const &authority);
};

std::string to_string(URLAuthority const &authority);

bool operator==(URLAuthority const &lhs, URLAuthority const &rhs);
bool operator<(URLAuthority const &lhs, URLAuthority const &rhs);

}

namespace std {

template<>
struct hash<TTauri::URLAuthority> {
    typedef TTauri::URLAuthority argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& authority) const noexcept {
        return
            std::hash<decltype(authority.userinfo)>{}(authority.userinfo) ^
            std::hash<decltype(authority.host)>{}(authority.host) ^
            std::hash<decltype(authority.port)>{}(authority.port);
    }
};

}