// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Path.hpp"

namespace TTauri::Draw {

namespace impl {

struct Fixed_buf_t {
    big_uint32_buf_t x;
    float value() const { return static_cast<float>(x.value()) / 65536.0f; }
};

struct shortFrac_buf_t {
    big_int16_buf_t x;
    float value() const { return static_cast<float>(x.value()) / 32768.0f; }
};

struct FWord_buf_t {
    big_int16_buf_t x;
    float value(uint16_t unitsPerEm) const { return static_cast<float>(x.value()) / static_cast<float>(unitsPerEm); }
};

struct FByte_buf_t {
    int8_t x;
    float value(uint16_t unitsPerEm) const { return static_cast<float>(x) / static_cast<float>(unitsPerEm); }
};

struct uFWord_buf_t {
    big_uint16_buf_t x;
    float value(uint16_t unitsPerEm) const { return static_cast<float>(x.value()) / static_cast<float>(unitsPerEm); }
};


struct GLYFEntry {
    big_int16_buf_t numberOfContours;
    FWord_buf_t xMin;
    FWord_buf_t yMin;
    FWord_buf_t xMax;
    FWord_buf_t yMax;
};

struct CMAPHeader {
    big_uint16_buf_t version;
    big_uint16_buf_t numTables;
};

struct CMAPEntry {
    big_uint16_buf_t platformID;
    big_uint16_buf_t platformSpecificID;
    big_uint32_buf_t offset;
};

struct CMAPFormat {
    big_uint16_buf_t format;
};

struct CMAPFormat4 {
    big_uint16_buf_t format;
    big_uint16_buf_t length;
    big_uint16_buf_t language;
    big_uint16_buf_t segCountX2;
    big_uint16_buf_t searchRange;
    big_uint16_buf_t entrySelector;
    big_uint16_buf_t rangeShift;
};

struct CMAPFormat6 {
    big_uint16_buf_t format;
    big_uint16_buf_t length;
    big_uint16_buf_t language;
    big_uint16_buf_t firstCode;
    big_uint16_buf_t entryCount;
};

struct CMAPFormat12 {
    big_uint32_buf_t format;
    big_uint32_buf_t length;
    big_uint32_buf_t language;
    big_uint32_buf_t numGroups;
};

struct CMAPFormat12Group {
    big_uint32_buf_t startCharCode;
    big_uint32_buf_t endCharCode;
    big_uint32_buf_t startGlyphCode;
};

struct HEADTable {
    Fixed_buf_t version;
    Fixed_buf_t fontRevision;
    big_uint32_buf_t checkSumAdjustment;
    big_uint32_buf_t magicNumber;
    big_uint16_buf_t flags;
    big_uint16_buf_t unitsPerEm;
    big_uint64_buf_t created;
    big_uint64_buf_t modified;
    FWord_buf_t xMin;
    FWord_buf_t yMin;
    FWord_buf_t xMax;
    FWord_buf_t yMax;
    big_uint16_buf_t macStyle;
    big_uint16_buf_t lowestRecPPEM;
    big_int16_buf_t fontDirectionHint;
    big_int16_buf_t indexToLocFormat;
    big_int16_buf_t glyphDataFormat;
};

struct HHEATable {
    Fixed_buf_t version;
    FWord_buf_t ascender;
    FWord_buf_t descender;
    FWord_buf_t lineGap;
    uFWord_buf_t advanceWidthMax;
    FWord_buf_t minLeftSideBearing;
    FWord_buf_t minRightSideBearing;
    FWord_buf_t xMaxExtent;
    big_int16_buf_t caretSlopeRise;
    big_int16_buf_t caretSlopRun;
    big_int16_buf_t caretOffset;
    big_int16_buf_t reserved0;
    big_int16_buf_t reserved1;
    big_int16_buf_t reserved2;
    big_int16_buf_t reserved3;
    big_int16_buf_t metricDataFormat;
    big_int16_buf_t numberOfHMetrics;
};

struct HMTXEntry {
    uFWord_buf_t advanceWidth;
    FWord_buf_t leftSideBearing;
};

struct MAXPTable {
    big_uint32_buf_t version;
    big_uint16_buf_t numGlyphs;
    big_uint16_buf_t maxPoints;
    big_uint16_buf_t maxContours;
    big_uint16_buf_t maxComponentPoints;
    big_uint16_buf_t maxComponentContours;
    big_uint16_buf_t maxZones;
    big_uint16_buf_t maxTwilightPoints;
    big_uint16_buf_t maxStorage;
    big_uint16_buf_t maxFunctionDefs;
    big_uint16_buf_t maxInstructionDefs;
    big_uint16_buf_t maxStackElements;
    big_uint16_buf_t maxSizeOfInstructions;
    big_uint16_buf_t maxComponentElements;
    big_uint16_buf_t maxComponentDepth;
};

}

class TrueTypeFont {
private:
    gsl::span<std::byte const> bytes;

    //! 'cmap' character to glyph mapping
    gsl::span<std::byte const> cmapTableBytes;

    //! 'glyf' glyph data
    gsl::span<std::byte const> glyfTableBytes;

    //! 'head' font header
    gsl::span<std::byte const> headTableBytes;

    //! 'hhea' horizontal header
    gsl::span<std::byte const> hheaTableBytes;

    //! 'hmtx' horizontal metrics
    gsl::span<std::byte const> hmtxTableBytes;

    //! 'loca' index to location
    gsl::span<std::byte const> locaTableBytes;

    //! 'maxp' maximum profile
    gsl::span<std::byte const> maxpTableBytes;

    //! 'name' naming (not needed)
    gsl::span<std::byte const> nameTableBytes;

    //! 'post' PostScript (not needed)
    gsl::span<std::byte const> postTableBytes;

public:
    TrueTypeFont(gsl::span<std::byte const> bytes);

    /*! Find a glyph in the font based on an unicode code-point.
     * This is seperated from loading a glyph so that graphemes and ligatures can be found.
     *
     * \param c Unicode code point to look up.
     * \return a glyph-index if a glyph has been found.
     */
    std::optional<int> findGlyphIndex(char32_t c);

    /*! Load a glyph into a path.
     * The glyph is directly loaded from the font fie.
     * 
     * \param glyphIndex the index of a glyph inside the font.
     * \return A path representing the glyph.
     */
    Path loadGlyph(int glyphIndex);

private:
    void parseFontDirectory();

};

}

