

target_sources(ttauri PRIVATE
    AttributedGlyph.cpp
    AttributedGlyph.hpp
    AttributedGlyphLine.hpp
    AttributedGrapheme.hpp
    code_point_iterator.hpp
    EditableText.hpp
    ElusiveIcons.hpp
    Font.cpp
    Font.hpp
    font_book.cpp
    font_book.hpp
    FontDescription.hpp
    FontFamilyID.hpp
    FontGlyphIDs.cpp
    FontGlyphIDs.hpp
    FontGraphemeID.hpp
    FontID.hpp
    FontVariant.hpp
    FontWeight.hpp
    GlyphID.hpp
    GlyphMetrics.hpp
    Grapheme.cpp
    Grapheme.hpp
    grapheme_iterator.hpp
    gstring.cpp
    gstring.hpp
    language.cpp
    language.hpp
    $<${TT_MACOS}:${CMAKE_CURRENT_SOURCE_DIR}/language_macos.mm>
    $<${TT_WIN32}:${CMAKE_CURRENT_SOURCE_DIR}/language_win32.cpp>
    po_parser.cpp
    po_parser.hpp
    ShapedText.cpp
    ShapedText.hpp
    TextDecoration.hpp
    TextStyle.cpp
    TextStyle.hpp
    translation.cpp
    translation.hpp
    TrueTypeFont.cpp
    TrueTypeFont.hpp
    TTauriIcons.hpp
    unicode_bidi.cpp
    unicode_bidi.hpp
    unicode_data.cpp
    unicode_data.hpp
    unicode_db.cpp
    unicode_db.hpp
    UnicodeRanges.cpp
    UnicodeRanges.hpp
)


target_sources(ttauri_tests PRIVATE
    unicode_data_tests.cpp
)
