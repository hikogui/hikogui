#!/bin/bash

if [ ! -e data/UnicodeData.txt ]
then
    echo "This script must be run from the ttauri root"
    exit 2
fi

python tools/unicode_data_generator.py \
    --output=src/ttauri/text/unicode_db.hpp \
    --unicode-data=data/UnicodeData.txt \
    --emoji-data=data/emoji-data.txt \
    --composition-exclusions=data/CompositionExclusions.txt \
    --grapheme-break-property=data/GraphemeBreakProperty.txt \
    --bidi-brackets=data/BidiBrackets.txt \
    --bidi-mirroring=data/BidiMirroring.txt

