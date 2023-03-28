#!/bin/bash

UCDDIR="data/ucd"

if [ ! -e ${UCDDIR}/UnicodeData.txt ]
then
    echo "This script must be run from the HikoGUI root"
    exit 2
fi

PYTHON3=python
if [ -e /usr/local/bin/python3 ]
then
    PYTHON3=/usr/local/bin/python3
fi


$PYTHON3 tools/generate_unicode_data.py \
    --compositions-template=tools/ucd/ucd_compositions.hpp.psp \
    --compositions-output=src/hikogui/unicode/ucd_compositions.hpp \
    --decompositions-template=tools/ucd/ucd_decompositions.hpp.psp \
    --decompositions-output=src/hikogui/unicode/ucd_decompositions.hpp \
    --general-categories-template=tools/ucd/ucd_general_categories.hpp.psp \
    --general-categories-output=src/hikogui/unicode/ucd_general_categories.hpp \
    --lexical-classes-template=tools/ucd/ucd_lexical_classes.hpp.psp \
    --lexical-classes-output=src/hikogui/unicode/ucd_lexical_classes.hpp \
    --scripts-template=tools/ucd/ucd_scripts.hpp.psp \
    --scripts-output=src/hikogui/unicode/ucd_scripts.hpp \
    --east-asian-widths-template=tools/ucd/ucd_east_asian_widths.hpp.psp \
    --east-asian-widths-output=src/hikogui/unicode/ucd_east_asian_widths.hpp \
    --bidi-classes-template=tools/ucd/ucd_bidi_classes.hpp.psp \
    --bidi-classes-output=src/hikogui/unicode/ucd_bidi_classes.hpp \
    --bidi-paired-bracket-types-template=tools/ucd/ucd_bidi_paired_bracket_types.hpp.psp \
    --bidi-paired-bracket-types-output=src/hikogui/unicode/ucd_bidi_paired_bracket_types.hpp \
    --bidi-mirroring-glyphs-template=tools/ucd/ucd_bidi_mirroring_glyphs.hpp.psp \
    --bidi-mirroring-glyphs-output=src/hikogui/unicode/ucd_bidi_mirroring_glyphs.hpp \
    --grapheme-cluster-breaks-template=tools/ucd/ucd_grapheme_cluster_breaks.hpp.psp \
    --grapheme-cluster-breaks-output=src/hikogui/unicode/ucd_grapheme_cluster_breaks.hpp \
    --line-break-classes-template=tools/ucd/ucd_line_break_classes.hpp.psp \
    --line-break-classes-output=src/hikogui/unicode/ucd_line_break_classes.hpp \
    --word-break-properties-template=tools/ucd/ucd_word_break_properties.hpp.psp \
    --word-break-properties-output=src/hikogui/unicode/ucd_word_break_properties.hpp \
    --sentence-break-properties-template=tools/ucd/ucd_sentence_break_properties.hpp.psp \
    --sentence-break-properties-output=src/hikogui/unicode/ucd_sentence_break_properties.hpp \
    --canonical-combining-classes-template=tools/ucd/ucd_canonical_combining_classes.hpp.psp \
    --canonical-combining-classes-output=src/hikogui/unicode/ucd_canonical_combining_classes.hpp \
    --index-template=tools/ucd/ucd_index.hpp.psp \
    --index-output=src/hikogui/unicode/ucd_index.hpp \
    --descriptions-template=tools/ucd/ucd_descriptions.hpp.psp \
    --descriptions-output=src/hikogui/unicode/ucd_descriptions.hpp \
    --bidi-brackets=${UCDDIR}/BidiBrackets.txt \
    --bidi-mirroring=${UCDDIR}/BidiMirroring.txt \
    --composition-exclusions=${UCDDIR}/CompositionExclusions.txt \
    --east-asian-width=${UCDDIR}/EastAsianWidth.txt \
    --emoji-data=${UCDDIR}/emoji-data.txt \
    --grapheme-break-property=${UCDDIR}/GraphemeBreakProperty.txt \
    --line-break=${UCDDIR}/LineBreak.txt \
    --prop-list=${UCDDIR}/PropList.txt \
    --scripts=${UCDDIR}/Scripts.txt \
    --sentence-break-property=${UCDDIR}/SentenceBreakProperty.txt \
    --unicode-data=${UCDDIR}/UnicodeData.txt \
    --word-break-property=${UCDDIR}/WordBreakProperty.txt

#python tools/unicode_data_generator.py \
#    --output=src/hikogui/unicode/unicode_db.hpp \
#    --unicode-data=${UCDDIR}/UnicodeData.txt \
#    --emoji-data=${UCDDIR}/emoji-data.txt \
#    --composition-exclusions=${UCDDIR}/CompositionExclusions.txt \
#    --grapheme-break-property=${UCDDIR}/GraphemeBreakProperty.txt \
#    --bidi-brackets=${UCDDIR}/BidiBrackets.txt \
#    --bidi-mirroring=${UCDDIR}/BidiMirroring.txt \
#    --line-break=${UCDDIR}/LineBreak.txt \
#    --word-break=${UCDDIR}/WordBreakProperty.txt \
#    --sentence-break=${UCDDIR}/SentenceBreakProperty.txt \
#    --east-asian-width=${UCDDIR}/EastAsianWidth.txt \
#    --scripts=${UCDDIR}/Scripts.txt

