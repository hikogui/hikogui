// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file file/URL.hpp Defines the URL class.
 * @ingroup file
 */

module;
#include "../macros.hpp"

#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <ostream>
#include <mutex>
#include <filesystem>

export module hikogui_path_URL;
import hikogui_char_maps;
import hikogui_path_URI;
import hikogui_path_path_location;
import hikogui_utility;

hi_warning_push();
// C26434: Function '' hides a non-virtual function ''.
// False positive reported: https://developercommunity.visualstudio.com/t/C26434-false-positive-with-conversion-op/10262199
hi_warning_ignore_msvc(26434);

export namespace hi { inline namespace v1 {

/** Universal Resource Locator.
 * @ingroup file
 *
 * An instance internally holds the URI split into its non-encoded components.
 *
 * syd::filesystem::path constructors will do context aware normalizations.
 *
 * 'file:' scheme urls can handle the following:
 *  - May contain a server name (placed in either the authority, or path of the url).
 *  - May contain a drive-letter.
 *  - May be absolute or relative, including proper handling of relative path with a named drive.
 *
 * The URL instance may be relative itself; meaning it does not hold a scheme.
 *
 * URLs can be implicitly converted to std::filesystem::path.
 */
class URL : public URI {
public:
    /** Create an empty URL.
     */
    constexpr URL() noexcept = default;
    constexpr URL(URL const&) noexcept = default;
    constexpr URL(URL&&) noexcept = default;
    constexpr URL& operator=(URL const&) noexcept = default;
    constexpr URL& operator=(URL&&) noexcept = default;

    /** Convert a URI to an URL.
     */
    constexpr explicit URL(URI const& other) noexcept : URI(other) {}

    /** Convert a URI to an URL.
     */
    constexpr explicit URL(URI&& other) noexcept : URI(std::move(other)){};

    /** Construct a URI from a string.
     *
     * @note This constructor will normalize the URI
     * @param str A URI encoded as a string.
     * @throws uri_error When the URI can not be normalized due to a parse error.
     */
    constexpr explicit URL(std::string_view str) : URI(str) {}

    /** Construct a URI from a string.
     *
     * @note This constructor will normalize the URI
     * @param str A URI encoded as a string.
     * @throws uri_error When the URI can not be normalized due to a parse error.
     */
    constexpr explicit URL(std::string const& str) : URL(std::string_view{str}) {}

    /** Construct a URI from a string.
     *
     * @note This constructor will normalize the URI
     * @param str A URI encoded as a string.
     * @throws uri_error When the URI can not be normalized due to a parse error.
     */
    constexpr explicit URL(const char *str) : URL(std::string_view{str}) {}

    /** Convert a filesystem-path to a file-scheme URL.
     *
     * A relative path is converted to a relative URL.
     * An absolute path is converted to an absolute file-scheme URL.
     *
     * @param path The path to convert to an URL.
     */
    explicit URL(std::filesystem::path const& path) : URI(make_file_url_string(path)) {}

    /** Return a generic path.
     *
     * @return The generic path of a file URL.
     * @throw url_error When a valid file path can not be constructed from the URL.
     */
    [[nodiscard]] constexpr std::u8string filesystem_path_generic_u8string(bool validate_scheme = true) const
    {
        if (validate_scheme and not(not scheme() or scheme() == "file")) {
            throw url_error("URL::generic_path() is only valid on a file: scheme URL");
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
        hi_assert(has_root_name == false or p.absolute());

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
        while (it != last) {
            validate_file_segment(*it);
            empty_segment = it->empty();

            if (it == first and empty_segment) {
                // Skip leading '/' in front of drive letter.
                ++it;

            } else if (auto i = it->find(':'); i != std::string::npos) {
                // Found a drive letter.
                if (i != 1) {
                    throw url_error("file URL contains a device name which is a security issue.");
                }

                if (has_root_name or p.absolute()) {
                    r += it->front();
                    // Use $ when the drive letter is on a server.
                    r += has_root_name ? '$' : ':';
                    // Add potentially a relative segment after the driver letter.
                    r += it->substr(2);

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
                break;

            } else {
                // Found a normal directory or filename.
                break;
            }
        }

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

        return to_u8string(r);
    }

    /** Create a filesystem path from a file URL.
     *
     * @return The filesystem path of a file URL.
     * @throw url_error When a valid file path can not be constructed from the URL.
     */
    [[nodiscard]] std::filesystem::path filesystem_path() const
    {
        if (auto scheme_ = scheme()) {
            if (scheme_ == "resource") {
                // Always used std::u8string with std::filesystem::path.
                hilet ref = std::filesystem::path{filesystem_path_generic_u8string(false)};
                if (auto path = find_path(resource_dirs(), ref)) {
                    return *path;
                } else {
                    throw url_error(std::format("Resource {} not found.", to_string(*this)));
                }

            } else if (scheme_ == "file") {
                return {filesystem_path_generic_u8string()};
            } else {
                throw url_error("URL can not be converted to a filesystem path.");
            }
        } else {
            // relative path.
            return {filesystem_path_generic_u8string()};
        }
    }

    /** @see filesystem_path()
     */
    operator std::filesystem::path() const
    {
        return filesystem_path();
    }

    [[nodiscard]] constexpr friend URL operator/(URL const& base, URI const& ref) noexcept
    {
        return URL{up_cast<URI const&>(base) / ref};
    }

    [[nodiscard]] constexpr friend URL operator/(URL const& base, std::string_view ref) noexcept
    {
        return URL{up_cast<URI const&>(base) / ref};
    }

private:
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

    static std::string make_file_url_string(std::filesystem::path const& path)
    {
        auto r = std::u8string{};

        hilet root_name = path.root_name().generic_u8string();
        if (root_name.empty()) {
            // No root-name.
            if (not path.root_directory().empty()) {
                // An absolute path should start with the file: scheme.
                r += u8"file:" + path.root_directory().generic_u8string();
            } else {
                // A relative path should not be prefixed with a scheme.
                ;
            }

        } else if (hilet i = root_name.find(':'); i != std::string::npos) {
            if (i == 1) {
                // Root name is a drive-letter, followed by potentially a relative path.
                r += u8"file:///" + root_name + path.root_directory().generic_u8string();
            } else {
                throw url_error("Paths containing a device are not allowed to be converted to a URL.");
            }
        } else {
            // Root name is a server.
            r += u8"file://" + root_name + path.root_directory().generic_u8string();
            if (not path.root_directory().empty()) {
                throw url_error("Invalid path contains server name without a root directory.");
            }
        }

        return to_string(r + path.relative_path().generic_u8string());
    }
};

}} // namespace hi::v1

template<>
struct std::hash<hi::URL> {
    [[nodiscard]] size_t operator()(hi::URL const& rhs) const noexcept
    {
        return std::hash<hi::URI>{}(rhs);
    }
};

// XXX #617 MSVC bug does not handle partial specialization in modules.
export template<>
struct std::formatter<hi::URL, char> : std::formatter<hi::URI, char> {
    auto format(hi::URL const& t, auto& fc) const
    {
        return std::formatter<hi::URI, char>::format(t, fc);
    }
};

hi_warning_pop();
