// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/grapheme.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri {

grapheme::grapheme(std::u32string_view codePoints) noexcept :
    value(0)
{
    let codePoints_ = Foundation_globals->unicodeData->toNFC(codePoints);

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
        if (codePoints_.size() <= std::tuple_size_v<long_grapheme>) {
            value = create_pointer(codePoints_.data(), codePoints_.size());
        } else {
            value = (0x00'fffdULL << 1) | 1; // Replacement character.
        }
    }
}

[[nodiscard]] std::u32string grapheme::NFD() const noexcept {
    return Foundation_globals->unicodeData->toNFD(static_cast<std::u32string>(*this));
}

[[nodiscard]] std::u32string grapheme::NFKC() const noexcept {
    return Foundation_globals->unicodeData->toNFKC(static_cast<std::u32string>(*this));
}

[[nodiscard]] std::u32string grapheme::NFKD() const noexcept {
    return Foundation_globals->unicodeData->toNFKD(static_cast<std::u32string>(*this));
}


}
