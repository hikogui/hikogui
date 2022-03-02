# Copyright Take Vos 2020-2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


target_sources(ttauri PRIVATE
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
    language.cpp
    language.hpp
    po_parser.cpp
    po_parser.hpp
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
    translation.cpp
    translation.hpp
    true_type_font_coverage.cpp
    true_type_font.cpp
    true_type_font.hpp
    ttauri_icon.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        language_tag_tests.cpp
    )
endif()
