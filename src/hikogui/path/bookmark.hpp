

#pragma once

#include "path_location.hpp"
#include "../macros.hpp"
#include <string>
#include <string_view>
#include <filesystem>

hi_export_module(hikogui.path : bookmark)

hi_export namespace hi::inline v1 {

class bookmark {
public:
    ~bookmark();

    /** Create an empty bookmark.
     */
    bookmark();

    bookmark(bookmark const& other);
    bookmark(bookmark&& other);
    bookmark& operator=(bookmark const& other);
    bookmark& operator=(bookmark&& other);

    /** Deserialize a bookmark including sandbox-tokens.
     *
     * The serialization will include any sandbox-tokens needed
     * to access the file again in a different process without
     * user intervention.
     *
     * @param str The string retrieved from a preferences file.
     */
    [[nodiscard]] static bookmark deserialize(std::string_view str);

    /** Serialize a bookmark including sandbox-tokens.
     *
     * The serialization will include any sandbox-tokens needed
     * to access the file again in a different process without
     * user intervention.
     *
     * @return A string which can be stored in preferences file.
     */
    [[nodiscard]] std::string serialize() const;

    /** This file is located outside the sandbox.
     *
     * If the file is located outside the sandbox then the application
     * will need to use `start_access_through_sandbox()` before
     * accessing the file.
     *
     * @retval true The file is located outside the sandbox.
     * @retval false The file is located inside the sandbox and
     *               is freely accessable by the application.
     */
    [[nodiscard]] bool is_outside_sandbox() const noexcept;

    /** Call this function before accessing the file.
     *
     * @param ask If true then attempt is made to get a
     *            new sandbox token, if the current token is
     *            no longer valid. This may happen through
     *            a dialogue box presented to the user.
     * @retval true Access was successful.
     * @retval false The sandbox token is no longer valid.
     */
    [[nodiscard]] bool start_access_through_sandbox(bool ask) const;

    /** Call this function after access is completed on a file.
     *
     * @note Each time `start_access_through_sandbox()` is called
     *       it should be balanced with `stop_access_through_sandbox()`.
     */
    [[nodiscard]] void stop_access_through_sandbox() const;


    /** The bookmark is empty.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return _path.empty();
    }

    /** Clear the bookmark.
     */
    void clear() noexcept
    {
        _path.clear();
        _location = path_location::none;
    }

    [[nodiscard]] bool is_resolved() const noexcept
    {
        return _path.is_absolute();
    }

    /** Check if the path exists on this disk.
     */
    [[nodiscard]] bool exists() const noexcept
    {
        if (is_resolved()) {
            return std::filesystem::exists(_path);
        } else {
            return false;
        }
    }

    /** Resolve the bookmark to an actual path on the disk.
     *
     * When resolving a bookmark with a given language and scale, a file
     * without a language and scale needs to exist. This will help with
     * searching quickly.
     * 
     * Files should have the following naming convention:
     *  - <path>/<name>(-<language>)(@<scale>x).<ext>
     * 
     * @param tags When searching for a file, try to find a file
     *             matching the earliest of the languages-tags in the list.
     * @param scale When searching for a file, try to find a file matching
     *              the given scale.
     * @return A bookmark that points to an actual file. or an error.
     */
    [[nodiscard]] std::exception<bookmark, std::error_code> resolve(std::vector<std::string> const &tags = {}, int scale = 0) const
    {
        if (_path.empty()) {
            return std::unexpected{std::make_error_code(std::errc::invalid_argument)};
        }

        // If the path is absolute then it is already resolved, only need to check
        // if it actually exists. In the future we may want to look if the file
        // has moved by tracking its inode.
        if (_path.is_absolute()) {
            if (std::filesystem::exists(_path)) {
                return *this;
            } else {
                return std::unexpected{std::make_error_code(std::errc::no_such_file_or_directory)};
            }
        }

        // Find a file without a language and scale, in the location specified
        // by the bookmark.
        auto const resolved_path = [&]() -> std::filesystem::path {
            for (auto const directory : location_dirs(_location)) {
                auto const tmp = directory / _path;
                if (std::filesystem::exists(tmp)) {
                    return tmp;
                }
            }

            return {};
        }();

        if (resolved_path.empty()) {
            return std::unexpected{std::make_error_code(std::errc::no_such_file_or_directory)};
        }

        // We now have a viable candidate for the file. Now look for a more
        // specific file with the given language and scale.
        auto const filename = resolved_path.filename().string();
        auto const directory = resolved_path.parent_path();
        if (filename.empty() or filename == "." or filename == "..") {
            return {resolved_path, _location};
        }

        // Split the filename into stem and all extensions.
        // This is different from std::filesystem::path::stem() and
        // std::filesystem::path::extension() because these will split
        // on only the last extension.
        auto const [stem, ext] = [&]() -> std::pair<std::string, std::string> {
            auto const dot_i = filename.find('.');
            if (dot_i == std::string::npos) {
                return {filename, {}};
            } else {
                return {filename.substr(0, dot_i), filename.substr(dot_i)};
            }
        }();          

        // Search for the file with the given language and scale.
        // Language has priority over scale.
        for (auto const &tag : tags) {
            assert(not tag.empty());

            for (auto s = scale; s != 0; s /= 2) {
                auto const tmp = directory / std::format("{}-{}@{}x{}", stem, tag, s, ext);
                if (std::filesystem::exists(tmp)) {
                    return {tmp, _location};
                }
            }

            auto const tmp = directory / std::format("{}-{}", stem, tag, ext);
            if (std::filesystem::exists(tmp)) {
                return {tmp, _location};
            }
        }

        // Search through scales without language.
        for (auto s = scale; s != 0; s /= 2) {
            auto const tmp = directory / std::format("{}@{}x{}", stem, s, ext);
            if (std::filesystem::exists(tmp)) {
                return {tmp, _location};
            }
        }

        // No file with the given language and scale was found.
        // Use the default file that was initially found.
        return {resolved_path, _location};
    }

private:
    std::filesystem::path _path;
    path_location _location;
};

}

