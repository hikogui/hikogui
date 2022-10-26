# Copyright Take Vos 2022.
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


target_sources(hikogui PRIVATE
    grapheme.cpp
    grapheme.hpp
    gstring.cpp
    gstring.hpp
    ucd_compositions.hpp
    ucd_decompositions.hpp
    ucd_index.hpp
    ucd_descriptions.hpp
    unicode_bidi.cpp
    unicode_bidi.hpp
    unicode_bidi_bracket_type.hpp
    unicode_bidi_class.hpp
    unicode_break_opportunity.hpp
    unicode_decomposition_type.hpp
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
    unicode_sentence_break.hpp
    unicode_text_segmentation.cpp
    unicode_text_segmentation.hpp
    unicode_word_break.hpp
)

if(BUILD_TESTING)
    target_sources(hikogui_tests PRIVATE
        unicode_bidi_tests.cpp
        unicode_break_tests.cpp
        unicode_text_segmentation_tests.cpp
        unicode_normalization_tests.cpp
    )
endif()
