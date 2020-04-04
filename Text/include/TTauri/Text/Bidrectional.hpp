
#pragma once

namespace TTauri::Text {

struct BidiGraphemeData {
    BidirectionalClass charClass;
    int8_t embedingLevel;

    BidiGraphemeData(BidirectionalClass charClass) noexcept :
        charClass(charClass), embedingLevel(0) {}
};


struct BidiData {
    std::vector<BidiParagraphData> parData;
    std::vector<BidiGraphemeData> textData;

    template<typename It, typename Char32Func>
    BidiParagraphData(It first, It last, Char32Func char32func) noexcept {
        // Initialization.
        for (auto i = first; i != last; ++i) {
            char32_t c = char32func(*i);
            ttauri_assume(c <= 0x10'ffff);

            let charClass = Text_global->unicodeData->getBidirectionalClass(c);
            textData.emplace_back(charClass);
        }

        // P1 Split paragraphs, and keep a paragraph seperator after each paragraph.
        if (textInfo.back().charClass == BirectionalClass::B) {
            textInfo.emplace_back(BidirectionalClass::B);
        }

        // P2 For each paragraph find the first L, AL, R class.
        // skipping isolates
        auto parClass = BidirectionalClass::Unknown;
        auto isolateLevel = 0;
        for (auto &data: textData) {
            switch (data.charClass) {
            case BidirectionalClass::L:
            case BidirectionalClass::AL:
            case BidirectionalClass::R:
                if (isolateLevel == 0 && parClass != BidirectionalClass::Unknown) {
                    parClass = data.charClass;
                }
                break;
            case BidirectionalClass::LRI:
            case BidirectionalClass::RLI:
            case BidirectionalClass::FSI:
                ++isolateLevel;
                break;
            case BidirectionalClass::PDI:
                --isolateLevel;
                break;
            case BidirectionalClass::B:
                // P3 AL=1, R=1, L=0, Unknown=0
                parData.emplace_back(
                    (parClass == BidirectionalClass::AL || parClass == BidirectionalClass::R) ?  1 : 0
                );
                parClass = BidirectionalClass::Unknown;
                isolateLevel = 0;
                break;
            default:;
            }
        }
    }


};

template<typename It, typename Char32Func>
[[nodiscard]] BidiParagraphData bidiInitialization(It first, It last, Char32Func char32func) noexcept {
    BidiParagraphData data(last, first);

    ttauri_assume(last != first);

    data.textInfo
    ttauri_assume((last - 1)

    return data;
}

}


