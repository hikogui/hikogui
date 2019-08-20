// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "exceptions.hpp"
#include <fmt/format.h>
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <numeric>
#include <iostream>
#include <cctype>

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
#define TTAURI_URL_REG_NAME TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS
#define TTAURI_URL_HOST TTAURI_URL_REG_NAME "[]"

std::string url_encode(std::string const &input, std::string_view const unreservedCharacters=TTAURI_URL_UNRESERVED) noexcept;
std::string url_decode(std::string_view const &input, bool plusToSpace=false) noexcept;

class URL {
private:
    std::string value;

public:
    URL() = default;
    URL(char const *url) : value(url) {}
    URL(std::string url) : value(std::move(url)) {}

    size_t hash() const noexcept { return std::hash<std::string>{}(value); }
    std::string string() const noexcept { return value; }

    std::optional<std::string_view> scheme() const noexcept;
    std::optional<std::string> query() const noexcept;
    std::optional<std::string> fragment() const noexcept;

    std::string filename() const noexcept;
    std::string directory() const noexcept;
    std::string extension() const noexcept;
    std::vector<std::string> pathSegments() const noexcept;
    std::string path() const noexcept;
    std::string nativePath() const noexcept;
    std::wstring nativeWPath() const noexcept;

    bool isAbsolute() const noexcept;
    bool isRelative() const noexcept { return !isAbsolute(); }

    URL urlByAppendingPath(URL const &other) const noexcept;
    URL urlByRemovingFilename() const noexcept;

    static URL urlFromPath(std::string_view const path) noexcept;
    static URL urlFromWPath(std::wstring_view const path) noexcept;
    static URL urlFromCurrentWorkingDirectory() noexcept;
    static URL urlFromResourceDirectory() noexcept;
    static URL urlFromExecutableDirectory() noexcept;
    static URL urlFromExecutableFile() noexcept;
    static URL urlFromApplicationDataDirectory() noexcept;

private:
    std::string_view encodedPath() const noexcept;
    std::string_view encodedFilename() const noexcept;
    std::string_view encodedExtension() const noexcept;
    std::vector<std::string_view> encodedPathSegments() const noexcept;
    std::optional<std::string_view> encodedQuery() const noexcept;
    std::optional<std::string_view> encodedFragment() const noexcept;

    friend bool operator==(URL const &lhs, URL const &rhs) noexcept;
    friend bool operator<(URL const &lhs, URL const &rhs) noexcept;
};

inline bool operator==(URL const &lhs, URL const &rhs) noexcept { return lhs.value == rhs.value; }
inline bool operator<(URL const &lhs, URL const &rhs) noexcept { return lhs.value < rhs.value; }
inline bool operator>(URL const &lhs, URL const &rhs) noexcept { return rhs < lhs; }
inline bool operator!=(URL const &lhs, URL const &rhs) noexcept { return !(lhs == rhs); }
inline bool operator>=(URL const &lhs, URL const &rhs) noexcept { return !(lhs < rhs); }
inline bool operator<=(URL const &lhs, URL const &rhs) noexcept { return !(lhs > rhs); }
inline URL operator/(URL const &lhs, URL const &rhs) noexcept { return lhs.urlByAppendingPath(rhs); }
std::string to_string(URL const &url) noexcept { return url.string(); }

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
    size_t operator()(TTauri::URL const& url) const noexcept {
        return url.hash();
    }
};

}
