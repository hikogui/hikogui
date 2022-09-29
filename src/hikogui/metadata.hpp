// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "file/URL.hpp"
#include "semantic_version.hpp"
#include <string>

namespace hi::inline v1 {

/** Metadata for a library or application.
 */
class metadata {
public:
    /** The name of the application or library.
     * The name should be in slug-format, i.e. based
     * on the following case-sensitive regular expression: [a-z-][a-z0-9-]*
     */
    std::string name;

    /** Display name of the application or library.
     * A free text string, may contain spaces and capital letters and letters
     * from other languages. It is however used for file and directory names.
     */
    std::string display_name;

    /** Name of the vendor of the application or library.
     * Free text name of the vendor, may contain spaces and capital letters
     * and letters from different languages. However the vendor field will
     * be used to construct file and directory paths.
     */
    std::string vendor;

    /** The version number of the application or library.
     */
    semantic_version version;

    /** The copyright license used for distribution.
     * This is a spdx-license-identifier of the license, not the
     * full license text.
     */
    std::string license;

    /** The homepage of the application or library.
     */
    URL homepage;

    /** Description of the application or library.
     * This is a free-text description of the application of library.
     * should not be longer than a single paragraph.
     */
    std::string description;

    /** The global application metadata.
     *
     * This function returns a reference to
     * the global application metadata. The first time this function is called
     * the application name and display_name are set based on the name of the
     * executable.
     *
     * The application metadata is also used when opening the Vulkan API which
     * request the name of the application and version number.
     */
    [[nodiscard]] static metadata &application() noexcept;

    /** The global hikogui-library metadata.
     *
     * This returns a reference to the metadata of the current hikogui library.
     * It may be useful for an application to read the version number.
     */
    [[nodiscard]] static metadata const &library() noexcept;
};

} // namespace hi::inline v1
