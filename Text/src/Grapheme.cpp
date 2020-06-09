// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/Grapheme.hpp"
#include "TTauri/Text/globals.hpp"

namespace TTauri {

Grapheme::Grapheme(std::u32string_view codePoints) noexcept :
    value(0)
{
    let codePoints_ = unicodeData->toNFC(codePoints);

    switch (codePoints_.size()) {
    case 3:
        value |= (static_cast<uint64_t>(codePoints_[2] & 0x1f'ffff) << 43);
        [[fallthrough]];
    case 2:
        value |= (static_cast<uint64_t>(codePoints_[1] & 0x1f'ffff) << 22);
        [[fallthrough]];
    case 1:
        value |= (static_cast<uint64_t>(codePoints_[0] & 0x1f'ffff) << 1);
        [[fallthrough]];
    case 0:
        value |= 1;
        break;
    default:
        if (codePoints_.size() <= std::tuple_size_v<long_Grapheme>) {
            value = create_pointer(codePoints_.data(), codePoints_.size());
        } else {
            value = (0x00'fffdULL << 1) | 1; // Replacement character.
        }
    }
}

[[nodiscard]] std::u32string Grapheme::NFD() const noexcept {
    return unicodeData->toNFD(static_cast<std::u32string>(*this));
}

[[nodiscard]] std::u32string Grapheme::NFKC() const noexcept {
    return unicodeData->toNFKC(static_cast<std::u32string>(*this));
}

[[nodiscard]] std::u32string Grapheme::NFKD() const noexcept {
    return unicodeData->toNFKD(static_cast<std::u32string>(*this));
}


}
