// Copyright 2019 Pokitec
// All rights reserved.

#include "TrueTypeParser.hpp"
#include "Glyph.hpp"
#include "exceptions.hpp"
#include "TTauri/FileView.hpp"
#include "TTauri/utils.hpp"

#include <boost/endian/buffers.hpp>

#include <map>
#include <array>

using namespace boost::endian;

namespace TTauri::Draw {

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

struct GLYFEntry {
    big_int16_buf_t numberOfContours;
    big_uint16_buf_t xMin;
    big_uint16_buf_t yMin;
    big_uint16_buf_t xMax;
    big_uint16_buf_t yMax;
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

struct MAXPHeader {
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

static std::map<char32_t, uint32_t> parseCMAPFormat4(gsl::span<std::byte> bytes)
{
    let &entry = at<CMAPFormat4>(bytes, 0);
    let segCount = entry.segCountX2.value() / 2;

    let endCodesOffset = sizeof(CMAPFormat4);
    let endCodes = make_span<big_uint16_buf_t>(bytes, endCodesOffset, segCount);

    let startCodesOffset = endCodesOffset + 2*segCount + 2;
    let startCodes = make_span<big_uint16_buf_t>(bytes, startCodesOffset, segCount);

    let idDeltasOffset = startCodesOffset + 2*segCount;
    let idDeltas = make_span<big_uint16_buf_t>(bytes, idDeltasOffset, segCount);

    let idRangeIndiciesOffset = idDeltasOffset + 2 * segCount;
    let idRangeIndices = make_span<big_uint16_buf_t>(bytes, idRangeIndiciesOffset, segCount);

    std::map<char32_t, uint32_t> characterToGlyph;
    for (uint16_t segmentIndex = 0; segmentIndex < segCount; segmentIndex++) {
        let startCode = startCodes.at(segmentIndex).value();
        let endCode = endCodes.at(segmentIndex).value();
        let idDelta = idDeltas.at(segmentIndex).value();
        let idRangeIndex = idRangeIndices.at(segmentIndex).value();
        let idRangeIndexOffset = idRangeIndiciesOffset + 2*segmentIndex;

        for (char32_t c = startCode; c <= endCode; c++) {
            if (idRangeIndex == 0) {
                characterToGlyph[c] = c + idDelta;
            } else {
                let index = idRangeIndex + idRangeIndexOffset + 2*(c - startCode);
                characterToGlyph[c] = at<big_uint16_buf_t>(bytes, index).value();
            }
        }
    }

    return characterToGlyph;
}

static std::map<char32_t, uint32_t> parseCMAPFormat6(gsl::span<std::byte> bytes)
{
    let& entry = at<CMAPFormat6>(bytes, 0);
    let firstCode = entry.firstCode.value();
    let entryCount = entry.entryCount.value();

    let glyphIndexArrayOffset = sizeof(CMAPFormat6);
    let glyphIndexArray = make_span<big_uint16_buf_t>(bytes, glyphIndexArrayOffset, entryCount);

    std::map<char32_t, uint32_t> characterToGlyph;
    for (uint16_t entryIndex = 0; entryIndex < entryCount; entryIndex++) {
        characterToGlyph[entryIndex + firstCode] = glyphIndexArray.at(entryIndex).value();
    }
    return characterToGlyph;
}

static std::map<char32_t, uint32_t> parseCMAPFormat12(gsl::span<std::byte> bytes)
{
    let& entry = at<CMAPFormat12>(bytes, 0);

    let groups = make_span<CMAPFormat12Group>(bytes, sizeof(CMAPFormat12), entry.numGroups.value());

    std::map<char32_t, uint32_t> characterToGlyph;
    for (let& group: groups) {
        let startCharCode = group.startCharCode.value();
        let endCharCode = group.endCharCode.value();
        auto glyphCode = group.startGlyphCode.value();
        for (char32_t c = startCharCode; c <= endCharCode; c++) {
            characterToGlyph[c] = glyphCode++;
        }
    }
    return characterToGlyph;
}

static gsl::span<CMAPEntry>::iterator findBestCMAPEntry(gsl::span<CMAPEntry> entries)
{
    std::vector<std::pair<uint16_t, uint16_t>> bestPlatforms = {
        {0, 4},
        {0, 0}, {0, 1}, {0, 2}, {0, 3},
        {3, 10}, {3, 1}, {3, 0}
    };

    for (let [ID, specificID] : bestPlatforms) {
        let i = std::find_if(entries.begin(), entries.end(), [ID, specificID](let& x) {
            return x.platformID.value() == ID && x.platformSpecificID.value() == specificID;
        });

        if (i != entries.end()) {
            return i;
        }
    }
    return entries.end();
}

static std::map<char32_t, uint32_t> parseCMAP(gsl::span<std::byte> bytes)
{
    let& header = at<CMAPHeader>(bytes, 0);
    if (!(header.version.value() == 0)) {
        BOOST_THROW_EXCEPTION(TrueTypeError("cmap.version is not 0"));
    }

    let entries = make_span<CMAPEntry>(bytes, sizeof(CMAPHeader), header.numTables.value());
    
    let i = findBestCMAPEntry(entries);
    if (i == entries.end()) {
        BOOST_THROW_EXCEPTION(TrueTypeError("Could not find a proper unicode character map"));
    }

    let tableOffset = i->offset.value();
    let tableSpan = bytes.subspan(tableOffset, bytes.size() - tableOffset);

    let table = at<CMAPFormat>(tableSpan, 0);
    switch (table.format.value()) {
    case 4: return parseCMAPFormat4(tableSpan);
    case 6: return parseCMAPFormat6(tableSpan);
    case 12: return parseCMAPFormat12(tableSpan);
    default:
        BOOST_THROW_EXCEPTION(TrueTypeError((boost::format("Unexpected character map format %i") % table.format.value()).str()));
    }
}

static Glyph parseGlyph(gsl::span<std::byte> bytes, float scale)
{

}

static std::vector<Glyph> parseGLYF(gsl::span<std::byte> bytes, float scale)
{
    std::vector<Glyph> glyphs;

    size_t offset = 0;
    while (offset < static_cast<size_t>(bytes.size())) {
        let &entry = at<GLYFEntry>(bytes, offset);
        offset += sizeof (GLYFEntry);

        auto &glyph = glyphs.emplace_back();

        let numberOfContours = entry.numberOfContours.value();
        if (numberOfContours == 0) {
            // Empty glyph.

        } else if (numberOfContours >= 0) {
            // Simple glyph.
            let endPoints = make_span<big_uint16_buf_t>(bytes, offset, numberOfContours);
            let numberOfPoints = endPoints.at(numberOfContours - 1).value();
            offset += numberOfContours * sizeof(uint16_t);

            // Skip over the instructions.
            let instructionLength = at<big_uint16_buf_t>(bytes, offset).value();
            offset += sizeof(uint16_t) + instructionLength;

            // Extract all the flags.
            std::vector<uint8_t> flags;
            for (size_t pointNr = 0; pointNr < numberOfPoints;) {
                let flag = at<uint8_t>(bytes, offset);
                flags.push_back(flag);
                offset += sizeof(uint8_t);

                if (flag & 0x08) {
                    let repeat = at<uint8_t>(bytes, offset);
                    flags.push_back(repeat);
                    offset += sizeof(uint8_t);
                    pointNr += repeat;
                }
                pointNr += 1;
            }

            // Get xCoordinates
            std::vector<int16_t> xCoordinates;
            for (auto i = flags.begin(); i != flags.end(); ++i) {
                let flag = *i;
                let repeat = (flag & 0x08) ? *(++i) + 1 : 1;

                for (size_t i = 0; i < repeat; i++) {
                    switch (flag & 0x12) {
                    case 0x00: // long-vector, different.
                        xCoordinates.push_back(at<big_int16_buf_t>(bytes, offset).value());
                        offset += sizeof(int16_t);
                        break;
                    case 0x10: // Long-vector, same.
                        xCoordinates.push_back(0);
                        break;
                    case 0x02: // short-vector, positive.
                        xCoordinates.push_back(at<uint8_t>(bytes, offset));
                        offset += sizeof(uint8_t);
                        break;
                    case 0x12: // short-vector, negative.
                        xCoordinates.push_back(-static_cast<int16_t>(at<uint8_t>(bytes, offset)));
                        offset += sizeof(uint8_t);
                        break;
                    default:
                        abort();
                    }
                }
            }

            // Get yCoordinates
            std::vector<int16_t> yCoordinates;
            for (auto i = flags.begin(); i != flags.end(); ++i) {
                let flag = *i;
                let repeat = (flag & 0x08) ? *(++i) + 1 : 1;

                for (size_t i = 0; i < repeat; i++) {
                    switch (flag & 0x24) {
                    case 0x00: // long-vector, different.
                        yCoordinates.push_back(at<big_int16_buf_t>(bytes, offset).value());
                        offset += sizeof(int16_t);
                        break;
                    case 0x20: // Long-vector, same.
                        yCoordinates.push_back(0);
                        break;
                    case 0x04: // short-vector, positive.
                        yCoordinates.push_back(at<uint8_t>(bytes, offset));
                        offset += sizeof(uint8_t);
                        break;
                    case 0x24: // short-vector, negative.
                        yCoordinates.push_back(-static_cast<int16_t>(at<uint8_t>(bytes, offset)));
                        offset += sizeof(uint8_t);
                        break;
                    default:
                        abort();
                    }
                }
            }

            // Create absolute points
            int16_t x = 0;
            int16_t y = 0;
            size_t pointNr = 0;
            size_t contourNr = 0;
            size_t endPoint = endPoints.at(contourNr).value();
            std::vector<std::pair<glm::vec2, bool>> points;
            for (auto i = flags.begin(); i != flags.end(); ++i) {
                let flag = *i;
                let repeat = (flag & 0x08) ? *(++i) + 1 : 1;

                for (size_t i = 0; i < repeat; i++) {
                    let onCurve = (flag & 0x01) > 0;
                    glm::vec2 coord = {
                        static_cast<float>(x += xCoordinates.at(pointNr)) * scale,
                        static_cast<float>(y += yCoordinates.at(pointNr)) * scale
                    };
                    pointNr++;

                    points.emplace_back(coord, onCurve);

                    let isEndPoint = (pointNr == endPoint);
                    if (isEndPoint) {
                        endPoint = endPoints.at(++contourNr).value();
                        glyph.addContour(points);
                        points.clear();
                    }
                }
            }

        } else {
            // Compound glyph.
            let flags = at<big_uint16_buf_t>(bytes, offset).value();
            offset += sizeof(uint16_t);
            let glyphIndex = at<big_uint16_buf_t>(bytes, offset).value();
            offset += sizeof(uint16_t);

        }

        offset = align(offset, sizeof (uint16_t));
    }

    return glyphs;
}

static std::vector<gsl::span<std::byte>> parseLOCA(gsl::span<std::byte> bytes, gsl::span<std::byte> glyfBytes, size_t numberOfGlyphs, bool longFormat)
{
    std::vector<gsl::span<std::byte>> r;

    if (longFormat) {
        let longTable = make_span<big_uint32_buf_t>(bytes, 0, numberOfGlyphs);

        for (i = 0; i < (longTable.size() - 1); i++) {
            let offset = static_cast<size_t>(longtable.at(i).value());
            let size = static_cast<size_t>(longtable.at(i + 1).value()) - offset;
            r.emplace_back(glyfBytes.subspan(offset, size));
        }
    } else {
        let shortTable = make_span<big_uint16_buf_t>(bytes, 0, numberOfGlyphs);

        for (i = 0; i < (shortTable.size() - 1); i++) {
            let offset = static_cast<size_t>(shortTable.at(i).value()) * 2;
            let size = static_cast<size_t>(shortTable.at(i + 1).value() * 2) - offset;
            r.emplace_back(glyfBytes.subspan(offset, size));
        }
    }

    return r;
}

static SFNTEntry const &findEntryFromHeader(gsl::span<SFNTEntry> const entries, uint32_t tag)
{
    for (let &entry: entries) {
        if (entry.tag.value() == tag) {
            return entry;
        }
    }
    BOOST_THROW_EXCEPTION(TrueTypeError((boost::format("Could not find '%s' entry in header.") % fourcc_to_string(tag)).str()));
}

Font parseTrueTypeFile(gsl::span<std::byte> bytes)
{
    Font font;

    let &header = at<SFNTHeader>(bytes, 0);
    if (!(header.scalerType.value() == fourcc("true") || header.scalerType.value() == 0x00010000)) {
        BOOST_THROW_EXCEPTION(TrueTypeError("sfnt.scalerType is not 'true' or 0x00010000"));
    }

    let headerEntries = make_span<SFNTEntry>(bytes, sizeof(SFNTHeader), header.numTables.value());
    // required tables, tables are sorted alphabetically, but we need to read them in another order.
    // 'cmap'	character to glyph mapping
    // 'glyf'	glyph data
    // 'head'	font header
    // 'hhea'	horizontal header
    // 'hmtx'	horizontal metrics
    // 'loca'	index to location (not needed)
    // 'maxp'	maximum profile
    // 'name'	naming
    // 'post'	PostScript (not needed)

    let characterMapHeaderEntry = findEntryFromHeader(headerEntries, fourcc("cmap"));
    let characterMap = parseCMAP(bytes.subspan(characterMapHeaderEntry.offset.value(), characterMapHeaderEntry.length.value()));

    let memoryRequirementHeaderEntry = findEntryFromHeader(headerEntries, fourcc("maxp"));
    let memoryRequirements = at<MAXPHeader>(bytes, memoryRequirementHeaderEntry.offset.value());
    let numGlyphs = memoryRequirements.numGlyphs.value();

    let glyphHeaderEntry = findEntryFromHeader(headerEntries, fourcc("glyf"));
    let glyphTable = bytes.subspan(glyphHeaderEntry.offset.value(), glyphHeaderEntry.length.value());

    let longFormat = true;
    let glyphLocationHeaderEntry = findEntryFromHeader(headerEntries, fourcc("loca"));
    let glyphDataMap = parseLOCA(bytes.subspan(glyphLocationHeaderEntry.offset.value(), glyphLocationHeaderEntry.length.value()), glyphTable, numOfGlyphs, longFormat);

    let scale = 0.0;
    let glyphs = parseGLYF(bytes.subspan(glyphHeaderEntry.offset.value(), glyphHeaderEntry.length.value()), scale);



    return font;
}

Font parseTrueTypeFile(std::filesystem::path& path)
{
    let view = FileView(path);
    try {
        return parseTrueTypeFile(view.bytes);
    } catch (boost::exception &e) {
        e << boost::errinfo_file_name(path.string());
        throw;
    }
}

}
