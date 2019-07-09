// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "URLAuthority.hpp"
#include "URLPath.hpp"
#include "required.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <numeric>

namespace std::filesystem {
class path;
}

namespace TTauri {

#define TTAURI_URL_ALPHA "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define TTAURI_URL_DIGIT "0123456789"
#define TTAURI_URL_HEXDIGIT "0123456789abcdefABCDEF"
#define TTAURI_URL_UNRESERVED TTAURI_URL_ALPHA TTAURI_URL_DIGIT "-._~"
#define TTAURI_URL_GEN_DELIMS ":/?#[]@"
#define TTAURI_URL_SUB_DELIMS "!$&'()*+,;="
#define TTAURI_URL_PCHAR TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS ":@"

std::string url_encode(std::string const &input, std::string_view const unreservedCharacters=TTAURI_URL_UNRESERVED);
std::string url_decode(std::string_view const &input, bool plusToSpace=false);


struct URL {
    std::string scheme = {};
    std::optional<URLAuthority> authority = {};
    URLPath path = {};
    std::optional<std::string> query = {};
    std::optional<std::string> fragment = {};

    URL() = default;
    URL(std::string const &url);
    URL(std::string scheme, URLPath path);

    std::string string_path() const;
    std::wstring wstring_path() const;
    std::string const &filename() const;
    std::string extension() const;

    URL urlByAppendingPath(URL const &other) const;
};

std::string to_string(URL const &url);

bool operator==(URL const &lhs, URL const &rhs);
bool operator<(URL const &lhs, URL const &rhs);
inline bool operator>(URL const &lhs, URL const &rhs) { return rhs < lhs; }
inline bool operator!=(URL const &lhs, URL const &rhs) { return !(lhs == rhs); }
inline bool operator>=(URL const &lhs, URL const &rhs) { return !(lhs < rhs); }
inline bool operator<=(URL const &lhs, URL const &rhs) { return !(lhs > rhs); }

size_t file_size(URL const &url);

}

namespace std {

template<>
struct hash<TTauri::URL> {
    typedef TTauri::URL argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& url) const noexcept {
        return
            std::hash<decltype(url.scheme)>{}(url.scheme) ^
            std::hash<decltype(url.authority)>{}(url.authority) ^
            std::hash<decltype(url.path)>{}(url.path) ^
            std::hash<decltype(url.query)>{}(url.query) ^
            std::hash<decltype(url.fragment)>{}(url.fragment);
    }
};

}