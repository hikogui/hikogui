// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "URLAuthority.hpp"
#include "URLPath.hpp"
#include "required.hpp"
#include "exceptions.hpp"
#include <fmt/format.h>
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <numeric>
#include <iostream>

namespace boost::filesystem {
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

std::string url_encode(std::string const &input, std::string_view const unreservedCharacters=TTAURI_URL_UNRESERVED) noexcept;
std::string url_decode(std::string_view const &input, bool plusToSpace=false) noexcept;

struct URL {
    std::string scheme = {};
    std::optional<URLAuthority> authority = {};
    URLPath path = {};
    std::optional<std::string> query = {};
    std::optional<std::string> fragment = {};

    URL() = default;
    URL(char const *url);
    URL(std::string const &url);
    URL(std::string const scheme, URLPath path) noexcept;

    std::string path_string() const;
    std::wstring path_wstring() const;
    std::string const &filename() const;
    std::string extension() const;

    bool isAbsolute() const noexcept;
    bool isRelative() const noexcept;

    URL urlByAppendingPath(URL const &other) const noexcept;
    URL urlByRemovingFilename() const noexcept;

    static URL urlFromWin32Path(std::wstring_view const path) noexcept;
    static URL urlFromCurrentWorkingDirectory() noexcept;
    static URL urlFromResourceDirectory() noexcept;
    static URL urlFromExecutableDirectory() noexcept;
    static URL urlFromExecutableFile() noexcept;
};

std::string to_string(URL const &url) noexcept;

bool operator==(URL const &lhs, URL const &rhs) noexcept;
bool operator<(URL const &lhs, URL const &rhs) noexcept;
inline bool operator>(URL const &lhs, URL const &rhs) noexcept { return rhs < lhs; }
inline bool operator!=(URL const &lhs, URL const &rhs) noexcept { return !(lhs == rhs); }
inline bool operator>=(URL const &lhs, URL const &rhs) noexcept { return !(lhs < rhs); }
inline bool operator<=(URL const &lhs, URL const &rhs) noexcept { return !(lhs > rhs); }

inline URL operator/(URL const &lhs, URL const &rhs) noexcept { return lhs.urlByAppendingPath(rhs); }

inline std::ostream& operator<<(std::ostream& lhs, const URL& rhs)
{
    lhs << to_string(rhs);
    return lhs;
}

size_t file_size(URL const &url);

template <typename T>
inline T parseResource(URL const &location)
{
    not_implemented;
}

template <typename T>
gsl_suppress2(26489,lifetime.1)
inline T &getResource(URL const &location)
{
    static std::unordered_map<URL,T> resourceCache = {};

    let oldResource = resourceCache.find(location);
    if (oldResource != resourceCache.end()) {
        return oldResource->second;
    }

    [[maybe_unused]] let [newResource, dummy] = resourceCache.try_emplace(location, parseResource<T>(location));

    return newResource->second;
}

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
