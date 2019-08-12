// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TrueTypeFont.hpp"
#include <boost/endian/buffers.hpp>

namespace TTauri::Draw {

TrueTypeFont::TrueTypeFont(gsl::span<std::byte const> bytes) : bytes(bytes)
{
    parseFontDirectory();
    parseCharacterMapDirectory();
}

struct CMAPHeader {
    big_uint16_buf_t version;
    big_uint16_buf_t numTables;
};

struct CMAPEntry {
    big_uint16_buf_t platformID;
    big_uint16_buf_t platformSpecificID;
    big_uint32_buf_t offset;
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

int TrueTypeFont::searchCharacterMapFormat4(char32_t c)
{
    if (c > 0xffff) {
        // character value too high.
        return 0;
    }

    auto offset = 0;

    let header = at<CMAPFormat4>(cmapBytes, 0);
    offset += sizeof(CMAPFormat4);

    let segCount = header.segCountX2.value() / 2;

    let endCode = make_span<big_uint16_buf_t>(cmapBytes, offset, segCount);
    offset += segCount * sizeof(uint16_t);
    offset += sizeof(uint16_t); // reservedPad

    let startCode = make_span<big_uint16_buf_t>(cmapBytes, offset, segCount);
    offset += segCount * sizeof(uint16_t);

    let idDelta = make_span<big_uint16_buf_t>(cmapBytes, offset, segCount);
    offset += segCount * sizeof(uint16_t);

    // The glyphIdArray is included inside idRangeOffset.
    let idRangeOffset_count = (header.length.value - offset) / sizeof(uint16_t);
    let idRangeOffset = make_span<big_uint16_buf_t>(cmapBytes, offset, idRangeOffset_count);

    for (let i = 0; i < segCount; i++) {
        let endCode_ = endCode.at(i).value();
        if (c <= endCode_) {
            let startCode_ = startCode.at(i).value();
            if (c >= startCode_) {
                // Found the glyph.
                let idRangeOffset_ = idRangeOffset.at(i).value();
                if (idRangeOffset_ == 0) {
                    // Use modulo 65536 arithmatic.
                    let u16_c = static_cast<uint16_t>(c);
                    return to_int(idDelta.at(i).value() + u16_c);

                } else {
                    let charOffset = c - startCode_;
                    let glyphOffset = (idRangeOffset_ / 2) + charOffset + i;

                    let glyphIndex = idRangeOffset.at(glyphOffset);
                    if (glyphIndex == 0) {
                        return 0;
                    } else {
                        // Use modulo 65536 arithmatic.
                        return to_int(idDelta[i].value() + glyphIndex);
                    }
                }

            } else {
                // character outside of segment
                return 0;
            }
        }
    }
}

struct CMAPFormat6 {
    big_uint16_buf_t format;
    big_uint16_buf_t length;
    big_uint16_buf_t language;
    big_uint16_buf_t firstCode;
    big_uint16_buf_t entryCount;
};

int TrueTypeFont::searchCharacterMapFormat6(char32_t c)
{
    auto offset = 0;
    let header = at<CMAPFormat6>(cmapBytes, 0);
    offset += sizeof(CMAPFormat6);

    let firstCode = header.firstCode.value();
    let entryCount = header.entryCount.value();
    if (c < firstCode || c >= (firstCode + entryCount)) {
        // Character outside of range.
        return 0;
    }

    let glyphIndexArray = make_span<big_uint16_buf_t>(cmapBytes, offset, entryCount);

    let charOffset = c - firstCode;
    return glyphIndexArray.at(charOffset);
}

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

int TrueTypeFont::searchCharacterMapFormat12(char32_t c)
{
    auto offset = 0;
    let header = at<CMAPFormat12>(cmapBytes, 0);
    offset += sizeof(CMAPFormat6);

    let entries = make_span<CMAPFormat12Group>(cmapBytes, offset, header.numGroups.value());
    for (let &entry: entries) {
        if (c <= entry.endCharCode) {
            let startCode_ = entry.startCode.value();
            if (c >= startCode_) {
                let charOffset = c - startCode_;
                return entry.startGlyphCode + charOffset;
            } else {
                // Character not found.
                return 0;
            }
        }
    }
}

int TrueTypeFont::searchCharacterMap(char32_t c)
{
    let format = at<big_uint16_buf_t>(cmapBytes, 0);
    switch (format.value()) {
    case 4: return searchCharacterMapFormat4(c);
    case 6: return searchCharacterMapFormat6(c);
    case 12: return searchCharacterMapFormat12(c);
    default:
        BOOST_THROW_EXCEPTION(ParseError((boost::format("Unknown cmap format %i") % format.value()).str()));
    }
}

void TrueTypeFont::parseCharacterMapDirectory()
{
    int64_t offset = 0;

    parse_assert(cmapTableBytes.size() >= sizeof(CMAPHeader));
    let &header = at<CMAPHeader>(cmapTableBytes, offset);
    offset += sizeof<CMAPHeader>

    parse_assert(header.version.value() == 0);

    int64_t numTables = header.numTables.value();
    parse_assert(cmapTableBytes.size() >= offset + numTables * to_int64(sizeof(CMAPEntry)));
    let entries = make_span<CMAPEntry>(cmapTableBytes, sizeof(CMAPHeader), header.numTables.value());

    // Entries are ordered by platformID, then platformSpecificID.
    // This allows us to search reasonable quickly for the best entries.
    // The following order is searched: 0.4,0.3,0.2,0.1,3.10,3.1,3.0.
    CMAPEntry *bestEntry = nullptr;
    for (let &entry: entries) {
        switch (entries.platformID.value()) {
        case 0: {
                // Unicode.
                switch (entries.platformSpecificID.value()) {
                case 0: // Default
                case 1: // Version 1.1
                case 2: // ISO 10646 1993
                case 3: // Unicode 2.0 BMP-only
                case 4: // Unicode 2.0 non-BMP
                    // The best entry is the last one.
                    bestEntry = &entry;
                    break;
                default:
                    // Not interesting
                    break;
                }
            } break;

        case 3: {
                // Microsoft Windows
                switch (entries.platformSpecificID.value()) {
                case 0: // Symbol
                case 1: // Unicode 16-bit
                case 10: // Unicode 32-bit
                    // The best entry is the last one.
                    bestEntry = &entry;
                    break;
                default:
                    // Not unicode
                    break;
                }
            } break;
        default:
            break;
        }
    }

    parse_assert(bestEntry);

    let entry_offset = to_int64(entry.offset.value());
    parse_assert(cmapTableBytes.size() >= entry.offset);

    cmapBytes = cmapTableBytes.subspan(entry_offset, cmapTableBytes.size() - entry_offset);
}

Path TrueTypeFont::loadGlyph(int glyphIndex)
{

}

struct SFNTHeader {
    big_uint32_buf_t scalerType;
    big_uint16_buf_t numTables;
    big_uint16_buf_t searchRange;
    big_uint16_buf_t entrySelector;
    big_uint16_buf_t rangeShift;
};

struct SFNTEntry {
    big_uint32_buf_t tag;
    big_uint32_buf_t checkSum;
    big_uint32_buf_t offset;
    big_uint32_buf_t length;
};

void TrueTypeFont::parseFontDirectory()
{
    let &header = at<SFNTHeader>(bytes, 0);

    if (!(header.scalerType.value() == fourcc("true") || header.scalerType.value() == 0x00010000)) {
        BOOST_THROW_EXCEPTION(ParseError("sfnt.scalerType is not 'true' or 0x00010000"));
    }

    let entries = make_span<SFNTEntry>(bytes, sizeof(SFNTHeader), header.numTables.value());
    for (let &entry: entries) {
        int64_t offset = entry.offset.value();
        int64_t length = entry.length.value();

        if (offset + length > to_int64(bytes.size())) {
            BOOST_THROW_EXCEPTION(ParseError("sfnt table-entry is out of range"));
        }

        switch (entry.tag.value()) {
        case fourcc("cmap"): cmapTableBytes = bytes.subspan(entry.offset.value(), entry.length.value()); break;
        case fourcc("glyf"): glyfTableBytes = bytes.subspan(entry.offset.value(), entry.length.value()); break;
        case fourcc("head"): headTableBytes = bytes.subspan(entry.offset.value(), entry.length.value()); break;
        case fourcc("hhea"): hheaTableBytes = bytes.subspan(entry.offset.value(), entry.length.value()); break;
        case fourcc("hmtx"): hmtxTableBytes = bytes.subspan(entry.offset.value(), entry.length.value()); break;
        case fourcc("loca"): locaTableBytes = bytes.subspan(entry.offset.value(), entry.length.value()); break;
        case fourcc("maxp"): maxpTableBytes = bytes.subspan(entry.offset.value(), entry.length.value()); break;
        case fourcc("name"): nameTableBytes = bytes.subspan(entry.offset.value(), entry.length.value()); break;
        case fourcc("post"): postTableBytes = bytes.subspan(entry.offset.value(), entry.length.value()); break;
        default: break;
        }
    }
}

}

