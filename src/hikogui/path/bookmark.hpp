

#pragma once

#include "../macros.hpp"

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

    /** Deserialize a bookmark including sanbox-tokens.
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
    [[nodiscard]] bool empty() const noexcept;

    /** Clear the bookmark.
     */
    void clear();


    /** Check if the path exists on this disk.
     */
    [[nodiscard]] bool exists() const noexcept
    {

    }

    /** Resolve the bookmark to an actual path on the disk.
     *
     * @return A bookmark that points to an actual file.
     * @retval std::nullopt If no file matches the bookmark.
     */
    [[nodiscard]] std::optional<bookmark> resolve() const noexcept
    {

    }

private:
    std::filesystem::path _path;
};

}

