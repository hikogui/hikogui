

curl https://www.unicode.org/Public/UCD/latest/ucd/BidiBrackets.txt -o BidiBrackets.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/BidiMirroring.txt -o BidiMirroring.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/CompositionExclusions.txt -o CompositionExclusions.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/EastAsianWidth.txt -o EastAsianWidth.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/Scripts.txt -o Scripts.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt -o UnicodeData.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/emoji/emoji-data.txt -o emoji-data.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/LineBreak.txt -o LineBreak.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/GraphemeBreakProperty.txt -o GraphemeBreakProperty.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/SentenceBreakProperty.txt -o SentenceBreakProperty.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/WordBreakProperty.txt -o WordBreakProperty.txt

TEST_FILES="BidiCharacterTest.txt BidiTest.txt auxiliary/GraphemeBreakTest.txt auxiliary/LineBreakTest.txt NormalizationTest.txt auxiliary/SentenceBreakTest.txt auxiliary/WordBreakTest.txt"

curl https://www.unicode.org/Public/UCD/latest/ucd/BidiCharacterTest.txt -o ../../tests/data/BidiCharacterTest.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/BidiTest.txt -o ../../tests/data/BidiTest.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/NormalizationTest.txt -o ../../tests/data/NormalizationTest.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/GraphemeBreakTest.txt -o ../../tests/data/GraphemeBreakTest.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/LineBreakTest.txt -o ../../tests/data/LineBreakTest.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/SentenceBreakTest.txt -o ../../tests/data/SentenceBreakTest.txt
curl https://www.unicode.org/Public/UCD/latest/ucd/auxiliary/WordBreakTest.txt -o ../../tests/data/WordBreakTest.txt

