// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;

#include <cstddef>
#include <span>

export module hikogui_font_otype_GSUB;
import hikogui_geometry;
import hikogui_parser;
import hikogui_utility;

export namespace hi::inline v1 {

/** Compatible with version 1.1, all offsets start at the beginning of this header.
 */
struct GSUB_version_1_0 {
    big_uint16_t major_version;
    big_uint16_t minor_version;
    big_uint16_t script_list_offset;
    big_uint16_t feature_list_offset;
    big_uint16_t lookup_list_offset;
};

struct GSUB_ligature {

};

}
