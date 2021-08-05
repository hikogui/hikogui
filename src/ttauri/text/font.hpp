// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_metrics.hpp"
#include "font_glyph_ids.hpp"
#include "gstring.hpp"
#include "font_description.hpp"
#include "../graphic_path.hpp"
#include "../resource_view.hpp"
#include "../exception.hpp"
#include "../required.hpp"
#include "../URL.hpp"
#include <span>
#include <vector>
#include <map>

namespace tt {

/*! A font.
 * This class has information on how to shape text and
 * get glyphs consisting of bezier contours.
 */
class font {
public:
    /** The description is filled with information parsed from the font.
     */
    font_description description;

    font() = default;
    virtual ~font() = default;
    font(font const &) = delete;
    font &operator=(font const &) = delete;
    font(font &&) = delete;
    font &operator=(font &&) = delete;


    /** Get the glyph for a code-point.
     * @return glyph-id, or invalid when not found or error.
     */
    [[nodiscard]] virtual tt::glyph_id find_glyph(char32_t c) const noexcept = 0;

    /** Get the glyphs for a grapheme.
    * @return a set of glyph-ids, or invalid when not found or error.
    */
    [[nodiscard]] font_glyph_ids find_glyph(grapheme g) const noexcept;

    /*! Load a glyph into a path.
    * The glyph is directly loaded from the font file.
    * 
    * \param glyph_id the id of a glyph inside the font.
    * \param path The path constructed by the loader.
    * \return empty on failure, or the glyphID of the metrics to use.
    */
    virtual std::optional<tt::glyph_id> load_glyph(tt::glyph_id glyph_id, graphic_path &path) const noexcept = 0;

    /*! Load a glyph into a path.
    * The glyph is directly loaded from the font file.
    * 
    * \param glyph_id the id of a glyph inside the font.
    * \param metrics The metrics constructed by the loader.
    * \param lookahead_glyph_id The id of a glyph to the right, needed for kerning.
    * \return true on success, false on error.
    */
    virtual bool load_glyph_metrics(
        tt::glyph_id glyph_id,
        glyph_metrics &metrics,
        tt::glyph_id lookahead_glyph_id = tt::glyph_id{})
        const noexcept = 0;

};

}
