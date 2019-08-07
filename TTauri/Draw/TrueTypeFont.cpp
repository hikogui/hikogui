// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TrueTypeFont.hpp"
#include <boost/endian/buffers.hpp>

namespace TTauri::Draw {

TrueTypeFont::TrueTypeFont(gsl::span<std::byte const> bytes) : bytes(bytes)
{
    parseFontDirectory();
}

std::optional<int> TrueTypeFont::findGlyphIndex(char32_t c)
{


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
        switch (entry.tag.value()) {
        case fourcc("cmap"): cmapTableBytes = bytes.subspace(entry.offset.value(), entry.length.value()); break;
        case fourcc("glyf"): glyfTableBytes = bytes.subspace(entry.offset.value(), entry.length.value()); break;
        case fourcc("head"): headTableBytes = bytes.subspace(entry.offset.value(), entry.length.value()); break;
        case fourcc("hhea"): hheaTableBytes = bytes.subspace(entry.offset.value(), entry.length.value()); break;
        case fourcc("hmtx"): hmtxTableBytes = bytes.subspace(entry.offset.value(), entry.length.value()); break;
        case fourcc("loca"): locaTableBytes = bytes.subspace(entry.offset.value(), entry.length.value()); break;
        case fourcc("maxp"): maxpTableBytes = bytes.subspace(entry.offset.value(), entry.length.value()); break;
        case fourcc("name"): nameTableBytes = bytes.subspace(entry.offset.value(), entry.length.value()); break;
        case fourcc("post"): postTableBytes = bytes.subspace(entry.offset.value(), entry.length.value()); break;
        default: break;
        }
    }
}

}

