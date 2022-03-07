#!/bin/bash

UCDDIR="data/ucd"

if [ ! -e ${UCDDIR}/UnicodeData.txt ]
then
    echo "This script must be run from the ttauri root"
    exit 2
fi


python tools/unicode_data_generator.py \
    --output=src/ttauri/unicode/unicode_db.hpp \
    --output-non-starter=src/ttauri/unicode/unicode_db_non_starter.hpp \
    --unicode-data=${UCDDIR}/UnicodeData.txt \
    --emoji-data=${UCDDIR}/emoji-data.txt \
    --composition-exclusions=${UCDDIR}/CompositionExclusions.txt \
    --grapheme-break-property=${UCDDIR}/GraphemeBreakProperty.txt \
    --bidi-brackets=${UCDDIR}/BidiBrackets.txt \
    --bidi-mirroring=${UCDDIR}/BidiMirroring.txt \
    --line-break=${UCDDIR}/LineBreak.txt \
    --word-break=${UCDDIR}/WordBreakProperty.txt \
    --sentence-break=${UCDDIR}/SentenceBreakProperty.txt \
    --east-asian-width=${UCDDIR}/EastAsianWidth.txt \
    --scripts=${UCDDIR}/Scripts.txt

