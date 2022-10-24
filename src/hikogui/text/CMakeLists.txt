# Copyright Take Vos 2021-2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


target_sources(hikogui PRIVATE
    agrapheme.hpp
    agstring.cpp
    agstring.hpp
    elusive_icon.hpp
    font.cpp
    font.hpp
    font_book.cpp
    font_book.hpp
    font_family_id.hpp
    font_grapheme_id.hpp
    font_metrics.hpp
    font_variant.hpp
    font_weight.hpp
    glyph_id.hpp
    glyph_ids.cpp
    glyph_ids.hpp
    glyph_metrics.hpp
    line_metrics.hpp
    semantic_text_style.hpp
    text_cursor.hpp
    text_decoration.hpp
    text_selection.hpp
    text_shaper.cpp
    text_shaper.hpp
    text_shaper_char.cpp
    text_shaper_char.hpp
    text_shaper_line.cpp
    text_shaper_line.hpp
    text_style.cpp
    text_style.hpp
    true_type_font_coverage.cpp
    true_type_font.cpp
    true_type_font.hpp
    hikogui_icon.hpp
)

#if(BUILD_TESTING)
#    target_sources(hikogui_tests PRIVATE
#    )
#endif()
#