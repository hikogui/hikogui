// Copyright 2019 Pokitec
// All rights reserved.

#include "Text.hpp"
#include <utf8proc/utf8proc.h>

namespace TTauri::Draw {

Text::Text(std::string const& str, Theme const &theme)
{
    if (str.size() == 0) {
        return;
    }

    let normalizedUTF8 = normalizeNFC(str);
    let normalizedUTF32 = translateString<std::u32string>(normalizedUTF8);
    let noLigaturesUF32 = splitLigatures(normalizedUTF32);

    // Split string into graphemes. Together with formatting.
    GraphemeCluster cluster;
    utf8proc_int32_t breakState = 0;
    utf8proc_int32_t previousCodePoint = -1;
    TextStyle style;
    for (let currentCodePoint: noLigaturesUF32) {
        if (style.updateFromCodePoint(currentCodePoint)) {
            // Strip out style changing code points.
            continue;
        }

        if (previousCodePoint >= 0) {
            if (utf8proc_grapheme_break_stateful(previousCodePoint, currentCodePoint, &breakState)) {
                addGraphemeCluster(cluster);
            }
        }

        cluster.emplace_back(currentCodePoint, style);
        previousCodePoint = currentCodePoint;
    }
    addGraphemeCluster(cluster);

    // Normalize graphemes to smallest representation.

    // Find ligatures glyphs in selected font.

    // Find glyphs for each grapheme in selected font.

    // Try different normalization of graphene to find glyph in selected font.

    // Try different normalization of graphene to find glyph in fallback font.

    // Select missing glyph from selected font.
}

void Text::addGraphemeCluster(GraphemeCluster const &cluster)
{
    assert(cluster.size() > 0);

    if (cluster.size() == 1) {
        text.push_back(cluster.at(0));
    }

    for (size_t i = 0; i < clusters.size(); i++) {
        if (clusters.at(i) == cluster) {
            text.push_back(Grapheme::GraphemeFromClusterIndex(i, cluster.at(0).getStyle()));
        }
    }

    clusters.push_back(cluster);
    text.push_back(Grapheme::GraphemeFromClusterIndex(clusters.size() - 1, cluster.at(0).getStyle()));
}

void Text::render(PixelMap<uint32_t> pixels, glm::vec2 offset, float angle)
{

}


}