

target_sources(ttauri PRIVATE
    detail/observable_cast_format10.hpp
    AttributedGlyph.cpp
    AttributedGlyph.hpp
    AttributedGlyphLine.hpp
    AttributedGrapheme.hpp
    code_point_iterator.hpp
    EditableText.hpp
    ElusiveIcons.hpp
    Font.cpp
    Font.hpp
    FontBook.cpp
    FontBook.hpp
    FontDescription.hpp
    FontFamilyID.hpp
    FontGlyphIDs.cpp
    FontGlyphIDs.hpp
    FontGraphemeID.hpp
    FontID.hpp
    FontVariant.hpp
    FontWeight.hpp
    format.hpp
    format10.hpp
    GlyphID.hpp
    GlyphMetrics.hpp
    Grapheme.cpp
    Grapheme.hpp
    grapheme_iterator.hpp
    gstring.cpp
    gstring.hpp
    l10n.hpp
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
    UnicodeBidi.cpp
    UnicodeBidi.hpp
    UnicodeData.cpp
    UnicodeData.hpp
    UnicodeRanges.cpp
    UnicodeRanges.hpp
)


target_sources(ttauri_tests PRIVATE
    UnicodeData_tests.cpp
)
