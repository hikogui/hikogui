// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "semantic_version.hpp"
#include "URL.hpp"
#include "logger.hpp"
#include <string>
#include <atomic>

namespace tt {

/** Meta for a library or application.
 */
class metadata {
public:
    /** The name of the application or library.
     * The name should be in slug-format, i.e. based
     * on the following case-sensitive regular expression: [a-z-][a-z0-9-]*
     */
    std::string name = "unknown-application";

    /** Display name of the application or library.
     * A free text string, may contain spaces and capital letters and letters
     * from other languages. It is however used for file and directory names.
     */
    std::string display_name = "Unknown Application";
    
    /** Name of the vendor of the application or library.
     * Free text name of the vendor, may contain spaces and capital letters
     * and letters from different languages. However the vendor field will
     * be used to construct file and directory paths.
     */
    std::string vendor = "Unknown Vendor";

    /** The version number of the application or library.
     */
    semantic_version version;

    /** The copyright license used for distribution.
     * This is a spdx-license-identifier of the license, not the
     * full license text.
     */
    std::string license = "unknown-spdx";

    /** The homepage of the application or library.
     */
    URL homepage;

    /** Description of the application or library.
     * This is a free-text description of the application of library.
     * should not be longer than a single paragraph.
     */
    std::string description = "";
};

namespace detail {
inline std::atomic<bool> application_metadata_is_set = false;
inline metadata application_metadata;
extern metadata library_metadata;
} // namespace detail

[[nodiscard]] inline metadata const &application_metadata() noexcept
{
    if (!detail::application_metadata_is_set.load(std::memory_order::acquire)) {
        tt_log_fatal("Application did not call tt::set_application_metadata()");
    }
    return detail::application_metadata;
}

inline void set_application_metadata(metadata const &rhs) noexcept
{
    detail::application_metadata = rhs;
    detail::application_metadata_is_set.store(true, std::memory_order::release);
}

[[nodiscard]] inline metadata const &library_metadata() noexcept
{
    return detail::library_metadata;
}

} // namespace tt
