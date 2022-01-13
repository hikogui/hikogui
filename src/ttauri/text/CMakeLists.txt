# Copyright Take Vos 2020-2021.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


target_sources(ttauri PRIVATE
    attributed_glyph.cpp
    attributed_glyph.hpp
    attributed_glyph_line.hpp
    attributed_grapheme.hpp
    code_point_iterator.hpp
    editable_text.hpp
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
    grapheme.cpp
    grapheme.hpp
    grapheme_iterator.hpp
    gstring.cpp
    gstring.hpp
    language.cpp
    language.hpp
    $<${TT_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/language_macos.mm>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/language_win32.cpp>
    po_parser.cpp
    po_parser.hpp
    shaped_text.cpp
    shaped_text.hpp
    text_decoration.hpp
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
    true_type_font.cpp
    true_type_font.hpp
    ttauri_icon.hpp
    unicode_db.hpp
    unicode_bidi.cpp
    unicode_bidi.hpp
    unicode_bidi_bracket_type.hpp
    unicode_bidi_class.hpp
    unicode_composition.cpp
    unicode_composition.hpp
    unicode_description.cpp
    unicode_description.hpp
    unicode_east_asian_width.hpp
    unicode_general_category.hpp
    unicode_grapheme_cluster_break.hpp
    unicode_line_break.hpp
    unicode_mask.cpp
    unicode_mask.hpp
    unicode_normalization.cpp
    unicode_normalization.hpp
    unicode_text_segmentation.cpp
    unicode_text_segmentation.hpp
)

if(TT_BUILD_TESTS)
    target_sources(ttauri_tests PRIVATE
        language_tag_tests.cpp
        unicode_bidi_tests.cpp
        unicode_line_break_tests.cpp
        unicode_text_segmentation_tests.cpp
        unicode_normalization_tests.cpp
        language_tag_tests.cpp
    )
endif()
