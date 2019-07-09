// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

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

struct URLUserinfo {
    std::string username = {};
    std::optional<std::string> password = {};

    URLUserinfo() = default;
    URLUserinfo(std::string const &userinfo);
};

std::string to_string(URLUserinfo const &userinfo);

bool operator==(URLUserinfo const &lhs, URLUserinfo const &rhs);
bool operator<(URLUserinfo const &lhs, URLUserinfo const &rhs);

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

struct URLPath {
    bool absolute = false;
    std::vector<std::string> segments = {};

    URLPath() = default;
    URLPath(std::string const &path);
    URLPath::URLPath(std::filesystem::path const &path);
    static URLPath fromWin32Path(std::wstring_view const &path_wstring);

    std::string string_path() const;

    std::string const &filename() const;

    std::string extension() const;
};

std::string to_string(URLPath const &path);
size_t file_size(URLPath const &path);

bool operator==(URLPath const &lhs, URLPath const &rhs);
bool operator<(URLPath const &lhs, URLPath const &rhs);

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
struct hash<TTauri::URLUserinfo> {
    typedef TTauri::URLUserinfo argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& userinfo) const noexcept {
        return
            std::hash<decltype(userinfo.username)>{}(userinfo.username) ^
            std::hash<decltype(userinfo.password)>{}(userinfo.password);
    }
};

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

template<>
struct hash<TTauri::URLPath> {
    typedef TTauri::URLPath argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& path) const noexcept {
        auto h = std::accumulate(path.segments.begin(), path.segments.end(), static_cast<size_t>(0), [](auto a, auto x) {
            return a ^ std::hash<decltype(x)>{}(x);
        });

        return h ^ std::hash<decltype(path.absolute)>{}(path.absolute);
    }
};

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