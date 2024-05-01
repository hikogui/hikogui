// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../unicode/unicode.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <algorithm>
#include <utility>

hi_export_module(hikogui.font : glyph_id);

hi_export namespace hi { inline namespace v1 {

/** The identifier of a glyph in a font-file.
 * 
 * The following values are special:
 *  - 0 : unidentified character, the glyph is an empty box.
 *  - 65535 : reserved, may not be used in an open-type file, used here for empty.
 */
class glyph_id : public tagged_id<glyph_id, uint16_t, std::numeric_limits<uint16_t>::max()> {
public:
    using super = tagged_id<glyph_id, uint16_t, std::numeric_limits<uint16_t>::max()>;
    using super::super;
};

}}

template<>
struct std::hash<hi::glyph_id> {
    [[nodiscard]] size_t operator()(hi::glyph_id rhs) const noexcept
    {
        return std::hash<hi::glyph_id::super>{}(rhs);
    }
};

