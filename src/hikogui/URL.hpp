// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "assert.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ostream>
#include <mutex>

namespace tt::inline v1 {
struct url_parts;
class resource_view;

/*! Universal Resource Locator.
 *
 * An instance internally holds a string to an url.
 * This will have the following effects:
 *  - Performance of accessors may be slow due to having to parse the url multiple times.
 *  - The size of the URL instance is small and copies/moves are fast.
 *
 * Constructors and path manipulations will cause the url to be normalized:
 *  - Remove accidental concatenation of two slashes 'foo//bar' -> 'foo/bar'
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
 * This also means that non of the constructors and non of the methods will ever cause an error.
 *
 * meaningless-urls could still cause meaningless results when converted to a path.
 * But this is no different from having a meaningless path in the first place.
 */
class URL {
public:
    constexpr URL() noexcept = default;
    explicit URL(std::string_view url);
    explicit URL(char const *url);
    explicit URL(std::string const &url);
    explicit URL(url_parts const &parts);

    URL(URL const &other) noexcept : value(other.value)
    {
        tt_axiom(&other != this);
    }
    URL(URL &&other) noexcept = default;
    URL &operator=(URL const &other) noexcept
    {
        // Self-assignment is allowed.
        value = other.value;
        return *this;
    }

    URL &operator=(URL &&other) noexcept = default;

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return value.empty();
    }

    [[nodiscard]] std::size_t hash() const noexcept;

    [[nodiscard]] std::string_view scheme() const noexcept;

    [[nodiscard]] std::string query() const noexcept;

    [[nodiscard]] std::string fragment() const noexcept;

    [[nodiscard]] std::string filename() const noexcept;

    [[nodiscard]] std::string directory() const noexcept;

    [[nodiscard]] std::string nativeDirectory() const noexcept;

    [[nodiscard]] std::string extension() const noexcept;

    [[nodiscard]] std::vector<std::string> pathSegments() const noexcept;

    [[nodiscard]] std::string path() const noexcept;

    [[nodiscard]] std::string nativePath() const noexcept;

    [[nodiscard]] std::wstring nativeWPath() const noexcept;

    [[nodiscard]] bool isFileScheme() const noexcept
    {
        return scheme() == "file";
    }

    [[nodiscard]] bool isAbsolute() const noexcept;
    [[nodiscard]] bool isRelative() const noexcept;
    [[nodiscard]] bool isRootDirectory() const noexcept;

    [[nodiscard]] URL urlByAppendingPath(URL const &other) const noexcept;

    [[nodiscard]] URL urlByAppendingPath(std::string_view const other) const noexcept;
    [[nodiscard]] URL urlByAppendingPath(std::string const &other) const noexcept;
    [[nodiscard]] URL urlByAppendingPath(char const *other) const noexcept;

    [[nodiscard]] URL urlByAppendingPath(std::wstring_view const other) const noexcept;
    [[nodiscard]] URL urlByAppendingPath(std::wstring const &other) const noexcept;
    [[nodiscard]] URL urlByAppendingPath(wchar_t const *other) const noexcept;

    [[nodiscard]] URL urlByAppendingExtension(std::string_view other) const noexcept;
    [[nodiscard]] URL urlByAppendingExtension(std::string const &other) const noexcept;
    [[nodiscard]] URL urlByAppendingExtension(char const *other) const noexcept;

    [[nodiscard]] URL urlByRemovingFilename() const noexcept;

    /** Load a resource.
     * @return A pointer to a resource view.
     */
    [[nodiscard]] std::unique_ptr<resource_view> loadView() const;

    /*! Return new URLs by finding matching files.
     * Currently only works for file: scheme urls.
     *
     * The following wildcards are supported:
     *  - '*' Replaced by 0 or more characters.
     *  - '?' Replaced by 1 character.
     *  - '**' Replaced by 0 or more nested directories.
     *  - '[abcd]' Replaced by a single character from the set "abcd".
     *  - '{foo,bar}' Replaced by a string "foo" or "bar".
     */
    [[nodiscard]] std::vector<URL> urlsByScanningWithGlobPattern() const noexcept;

    [[nodiscard]] static URL urlFromPath(std::string_view const path) noexcept;
    [[nodiscard]] static URL urlFromWPath(std::wstring_view const path) noexcept;

    static void setUrlForCurrentWorkingDirectory(URL url) noexcept;
    [[nodiscard]] static URL urlFromCurrentWorkingDirectory() noexcept;
    [[nodiscard]] static URL urlFromResourceDirectory() noexcept;
    [[nodiscard]] static URL urlFromExecutableDirectory() noexcept;
    [[nodiscard]] static URL urlFromExecutableFile() noexcept;
    [[nodiscard]] static URL urlFromApplicationDataDirectory() noexcept;
    [[nodiscard]] static URL urlFromApplicationLogDirectory() noexcept;
    [[nodiscard]] static URL urlFromSystemfontDirectory() noexcept;
    [[nodiscard]] static URL urlFromApplicationPreferencesFile() noexcept;

    /*! Return file names in the directory pointed by the url.
     * \param path path to the directory to scan.
     * \return A list of filenames or subdirectories (ending in '/') in the directory.
     */
    [[nodiscard]] static std::vector<std::string> filenamesByScanningDirectory(std::string_view path) noexcept;

    [[nodiscard]] static std::string nativePathFromPath(std::string_view path) noexcept;
    [[nodiscard]] static std::wstring nativeWPathFromPath(std::string_view path) noexcept;

    [[nodiscard]] friend bool operator==(URL const &lhs, URL const &rhs) noexcept
    {
        return lhs.value == rhs.value;
    }

    [[nodiscard]] friend auto operator<=>(URL const &lhs, URL const &rhs) noexcept
    {
        return lhs.value <=> rhs.value;
    }

    [[nodiscard]] friend URL operator/(URL const &lhs, URL const &rhs) noexcept
    {
        return lhs.urlByAppendingPath(rhs);
    }

    [[nodiscard]] friend URL operator/(URL const &lhs, std::string_view const &rhs) noexcept
    {
        return lhs.urlByAppendingPath(URL{rhs});
    }

    [[nodiscard]] friend std::string const &to_string(URL const &url) noexcept
    {
        return url.value;
    }

    friend std::ostream &operator<<(std::ostream &lhs, const URL &rhs)
    {
        return lhs << to_string(rhs);
    }

private:
    std::string value;

    static URL _urlOfCurrentWorkingDirectory;
};

} // namespace tt::inline v1

template<>
class std::hash<tt::URL> {
public:
    std::size_t operator()(tt::URL const &url) const noexcept
    {
        return url.hash();
    }
};

template<typename CharT>
struct std::formatter<tt::URL, CharT> : std::formatter<std::string_view, CharT> {
    auto format(tt::URL const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};
