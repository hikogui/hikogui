// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "exceptions.hpp"
#include "url_parser.hpp"
#include "utils.hpp"
#include "strings.hpp"
#include <fmt/format.h>
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <numeric>
#include <iostream>

namespace TTauri {

class URL {
private:
    std::string value;

public:
    URL() = default;
    URL(std::string_view url) : value(normalize_url(url)) {}
    URL(char const *url) : value(normalize_url(url)) {}
    URL(std::string const &url) : value(normalize_url(url)) {}
    URL(url_parts const &parts) : value(generate_url(parts)) {}

    URL(URL const &other) = default;
    URL(URL &&other) = default;
    ~URL() = default;
    URL &operator=(URL const &other) = default;
    URL &operator=(URL &&other) = default;

    size_t hash() const noexcept { return std::hash<std::string>{}(value); }
    std::string string() const noexcept { return value; }

    std::string_view scheme() const noexcept {
        return parse_url(value).scheme;
    }

    std::string query() const noexcept {
        return url_decode(parse_url(value).query, true);
    }

    std::string fragment() const noexcept {
        return url_decode(parse_url(value).fragment);
    }

    std::string filename() const noexcept {
        let parts = parse_url(value);
        if (parts.segments.size() > 0) {
            return url_decode(parts.segments.back());
        } else {
            return {};
        }
    }

    std::string directory() const noexcept {
        auto parts = parse_url(value);
        if (parts.segments.size() > 0) {
            parts.segments.pop_back();
        }
        return generate_path(parts);
    }

    std::string nativeDirectory() const noexcept {
        auto parts = parse_url(value);
        if (parts.segments.size() > 0) {
            parts.segments.pop_back();
        }
        return generate_native_path(parts);
    }

    std::string extension() const noexcept {
        let fn = filename();
        let i = fn.rfind('.');
        return fn.substr((i != fn.npos) ? (i + 1) : fn.size());
    }

    std::vector<std::string> pathSegments() const noexcept {
        let parts = parse_url(value);
        return transform<std::vector<std::string>>(parts.segments, [](auto x) {
            return url_decode(x);
        });
    }

    std::string path() const noexcept {
        return generate_path(parse_url(value));
    }

    std::string nativePath() const noexcept {
        return generate_native_path(parse_url(value));
    }

    std::wstring nativeWPath() const noexcept {
        return translateString<std::wstring>(nativePath());
    }

    bool isAbsolute() const noexcept { return parse_url(value).absolute; }
    bool isRelative() const noexcept { return !isAbsolute(); }

    URL urlByAppendingPath(URL const &other) const noexcept {
        let this_parts = parse_url(value);
        let other_parts = parse_url(other.value);
        let new_parts = concatenate_url_parts(this_parts, other_parts);
        return URL(new_parts);
    }

    URL urlByAppendingPath(std::string_view const other) const noexcept {
        return urlByAppendingPath(URL::urlFromPath(other));
    }

    URL urlByAppendingPath(std::wstring_view const other) const noexcept {
        return urlByAppendingPath(URL::urlFromWPath(other));
    }

    URL urlByRemovingFilename() const noexcept {
        auto parts = parse_url(value);
        if (parts.segments.size() > 0) {
            parts.segments.pop_back();
        }
        return URL(parts);
    }

    static URL urlFromPath(std::string_view const path) noexcept {
        std::string tmp;
        let parts = parse_path(path, tmp);
        return URL(parts);
    }

    static URL urlFromWPath(std::wstring_view const path) noexcept {
        return urlFromPath(translateString<std::string>(path));
    }

    static URL urlFromCurrentWorkingDirectory() noexcept;
    static URL urlFromResourceDirectory() noexcept;
    static URL urlFromExecutableDirectory() noexcept;
    static URL urlFromExecutableFile() noexcept;
    static std::optional<URL> urlFromApplicationDataDirectory() noexcept;
    static std::optional<URL> urlFromApplicationLogDirectory() noexcept;

private:
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
inline std::string to_string(URL const &url) noexcept { return url.string(); }

inline std::ostream& operator<<(std::ostream& lhs, const URL& rhs)
{
    lhs << to_string(rhs);
    return lhs;
}

int64_t fileSize(URL const &url);

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
