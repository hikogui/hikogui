// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "assert.hpp"
#include "URI.hpp"
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ostream>
#include <mutex>
#include <filesystem>

namespace hi { inline namespace v1 {
class resource_view;

/** Universal Resource Locator.
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
class URL : public URI {
public:
    using URI::URI;

    constexpr void static validate_file_segment(std::string_view segment)
    {
        for (auto c : segment) {
            if (c == '/' or c == '\\') {
                throw url_error("Filename server name may not contain slash or back-slash.");
            }
        }
    }

    constexpr void static validate_file_server(std::string_view server)
    {
        for (auto c : server) {
            if (c == '/' or c == '\\') {
                throw url_error("Filename segments may not contain slash or back-slash.");
            }
        }
    }

    [[nodiscard]] std::filesystem::path filesystem_path() const
    {
        if (not (not scheme() or scheme() == "file")) {
            throw url_error("URL::filesystem_path() is only valid on a file: scheme URL");
        }

        auto r = std::string{};
        hilet& p = path();
        hilet first = p.begin();
        hilet last = p.end();
        auto it = first;

        auto has_root_name = false;
        if (authority()) {
            // file://server/filename is valid.
            auto server = to_string(*authority());
            if (not server.empty() and server != "localhost") {
                validate_file_server(server);
                has_root_name = true;
                r += '/';
                r += '/';
                r += server;
                r += '/';
            }
        }

        // If a server was found than the path must be absolute.
        hi_axiom(has_root_name == false or p.absolute());

        if (p.double_absolute()) {
            // file:////server/filename is valid.
            if (has_root_name) {
                // file://server//server/filename is invalid.
                throw url_error("file URL has two server names.");
            }

            has_root_name = true;
            r += '/';
            r += '/';
            it += 2;
            validate_file_server(*it);
            r += *it++;
            r += '/';
        }

        // Find the drive letter.
        auto empty_segment = false;
        do {
            if (it != last) {
                validate_file_segment(*it);
                empty_segment = it->empty();

                if (it == first and it->empty() and has_root_name) {
                    // If a root-name is already defined, skip the root-directory slash.
                    ++it;
                    continue;

                } else if (auto i = it->find(':'); i != std::string::npos) {
                    // Found a drive letter.
                    if (i != 1) {
                        throw url_error("file URL contains a device name which is a security issue.");
                    }

                    if (has_root_name or p.absolute()) {
                        r += it->front();
                        // Use $ when the drive letter is on a server.
                        r += has_root_name ? '$' : ':';

                        // If there is a root name, the directory after the drive letter must be absolute too.
                        if (it->size() > 2) {
                            r += '/';
                            r += it->substr(2);
                        }

                    } else {
                        // Take the drive letter and optional relative directory directly on relative paths.
                        // C:dirname valid
                        // C:/dirname valid
                        // file:C:dirname valid
                        // file:C:/dirname valid
                        r += *it;
                    }

                    has_root_name = true;
                    r += '/';
                    ++it;
                }
            }
        } while (false);

        // The rest are directories followed by a single (optionally empty) filename
        for (; it != last; ++it) {
            validate_file_segment(*it);
            empty_segment = it->empty();
            r += *it;
            r += '/';
        }

        if (not empty_segment) {
            // Remove trailing backslash if the last segment was a filename.
            r.pop_back();
        }

        return std::filesystem::path{std::move(r)};
    }

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

private:
    static URL _urlOfCurrentWorkingDirectory;
};

}} // namespace hi::v1

template<>
struct std::hash<hi::URL> {
    [[nodiscard]] size_t operator()(hi::URL const& rhs) const noexcept
    {
        return std::hash<hi::URI>{}(rhs);
    }
};

template<typename CharT>
struct std::formatter<hi::URL, CharT> : std::formatter<hi::URI, CharT> {
    auto format(hi::URL const& t, auto& fc)
    {
        return std::formatter<hi::URI, CharT>::format(t, fc);
    }
};
