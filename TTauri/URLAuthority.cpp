// Copyright 2019 Pokitec
// All rights reserved.

#include "URLAuthority.hpp"
#include "URL.hpp"
#include "exceptions.hpp"
#include <regex>

namespace TTauri {

const auto URLAuthority_re = std::regex{"^(([^@]*)@)?(([[][^]]*[]])|([^:]*))(:([0-9]+))?", std::regex::extended};
//                                        12         34             5       6 7

URLAuthority::URLAuthority(std::string const &authority)
{
    std::smatch m;

    if (!std::regex_match(authority.begin(), authority.end(), m, URLAuthority_re)) {
        TTAURI_THROW(url_error("Could not parse URLAuthority")
            << error_info("parse_string", authority)
        );
    }

    required_assert(m.ready());
    let userinfo_sm = m[2];
    let host_sm = m[3];
    let port_sm = m[7];

    if (userinfo_sm.length()) {
        userinfo = URLUserinfo{userinfo_sm.str()};
    }
    host = url_decode(host_sm.str());
    if (port_sm.length()) {
        port = port_sm.str();
    }
}

std::string to_string(URLAuthority const &authority) noexcept
{
    auto s = std::string{};

    if (authority.userinfo) {
        s += to_string(authority.userinfo.value()) + "@";
    }

    if (authority.host.front() == '[' && authority.host.back() == ']') {
        s += url_encode(authority.host, TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS "[]:");
    } else {
        s += url_encode(authority.host, TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS);
    }

    if (authority.port) {
        s += ":" + url_encode(authority.host, TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS);
    }

    return s;
}

bool operator==(URLAuthority const &lhs, URLAuthority const &rhs) noexcept
{
    return std::tie(lhs.userinfo, lhs.host, lhs.port) == std::tie(rhs.userinfo, rhs.host, rhs.port);
}

bool operator<(URLAuthority const &lhs, URLAuthority const &rhs) noexcept
{
    return std::tie(lhs.userinfo, lhs.host, lhs.port) < std::tie(rhs.userinfo, rhs.host, rhs.port);
}

}