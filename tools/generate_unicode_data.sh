#!/bin/bash

UCDDIR="data/ucd"

if [ ! -e ${UCDDIR}/UnicodeData.txt ]
then
    echo "This script must be run from the HikoGUI root"
    exit 2
fi


python tools/generate_unicode_data.py \
    --compositions-template=tools/ucd/ucd_compositions.hpp.psp \
    --compositions-output=src/hikogui/unicode/ucd_compositions.hpp \
    --decompositions-template=tools/ucd/ucd_decompositions.hpp.psp \
    --decompositions-output=src/hikogui/unicode/ucd_decompositions.hpp \
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

