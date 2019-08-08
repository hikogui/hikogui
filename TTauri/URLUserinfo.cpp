// Copyright 2019 Pokitec
// All rights reserved.

#include "URLUserinfo.hpp"
#include "URL.hpp"

namespace TTauri {

URLUserinfo::URLUserinfo(std::string const &userinfo) noexcept
{
    auto i = userinfo.find(':');
    if (i != std::string_view::npos) {
        username = url_decode(userinfo.substr(0, i));
        password = url_decode(userinfo.substr(i + 1));
    } else {
        username = url_decode(userinfo);
        password = {};
    }
}

std::string to_string(URLUserinfo const &userinfo) noexcept
{
    auto s = url_encode(userinfo.username, TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS);

    if (userinfo.password) {
        s += ":" + url_encode(userinfo.password.value(), TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS);
    }
    return s;
}

bool operator==(URLUserinfo const &lhs, URLUserinfo const &rhs) noexcept
{
    return std::tie(lhs.username, lhs.password) == std::tie(rhs.username, rhs.password);
}

bool operator<(URLUserinfo const &lhs, URLUserinfo const &rhs) noexcept
{
    return std::tie(lhs.username, lhs.password) < std::tie(rhs.username, rhs.password);
}

}