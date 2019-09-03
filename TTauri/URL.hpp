// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <unordered_map>

namespace TTauri {
struct url_parts;

/*! Universal Resource Locator.
 *
 * An instance internally holds a string to an url.
 * This will have the following effects:
 *  - Performance of accessors may be slow due to having to parse the url multiple times.
 *  - The size of the URL instance is small and copies/moves are fast.
 * 
 * Constructors and path manipulations will cause the url to be normalized:
 *  - Remove accidental concatination of two slashes 'foo//bar' -> 'foo/bar'
 *  - Remove single dot directories 'foo/./bar' -> 'foo/bar'
 *  - Remove leading double-dot directories on absolute paths '/../foo' -> '/foo'
 *  - Remove name+double-dot combinations 'foo/bar/../baz' -> 'foo/baz'
 *
 * 'file:' scheme urls can handle the following:
 *  - May contain a server name (placed in the authority of the url)
 *  - May contain a drive-letter.
 *  - May be absolute or relative, including proper handling of relative path with a named drive. 
 *
 * The url instance may be relative itself; meaning it does not hold a scheme.
 * This is important, because it means that any string passed to the constructor is a valid url.
 * This also means that non of the custructors and non of the methods will ever cause an error.
 *
 * meaningless-urls could still cause meaningless results when converted to a path.
 * But this is no different from having a meaningless path in the first place.
 */
class URL {
private:
    std::string value;

public:
    URL() = default;
    URL(std::string_view url);
    URL(char const *url);
    URL(std::string const &url);
    URL(url_parts const &parts);

    URL(URL const &other) = default;
    URL(URL &&other) = default;
    ~URL() = default;
    URL &operator=(URL const &other) = default;
    URL &operator=(URL &&other) = default;

    size_t hash() const noexcept;
    std::string string() const noexcept;

    std::string_view scheme() const noexcept;

    std::string query() const noexcept;

    std::string fragment() const noexcept;

    std::string filename() const noexcept;

    std::string directory() const noexcept;

    std::string nativeDirectory() const noexcept;

    std::string extension() const noexcept;

    std::vector<std::string> pathSegments() const noexcept;

    std::string path() const noexcept;

    std::string nativePath() const noexcept;

    std::wstring nativeWPath() const noexcept;

    bool isAbsolute() const noexcept;
    bool isRelative() const noexcept;

    URL urlByAppendingPath(URL const &other) const noexcept;

    URL urlByAppendingPath(std::string_view const other) const noexcept;

    URL urlByAppendingPath(std::wstring_view const other) const noexcept;

    URL urlByRemovingFilename() const noexcept;

    static URL urlFromPath(std::string_view const path) noexcept;

    static URL urlFromWPath(std::wstring_view const path) noexcept;

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

std::ostream& operator<<(std::ostream& lhs, const URL& rhs);

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
class hash<TTauri::URL> {
public:
    size_t operator()(TTauri::URL const& url) const noexcept {
        return url.hash();
    }
};

}
