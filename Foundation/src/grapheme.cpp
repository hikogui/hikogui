// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/grapheme.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri {

[[nodiscard]] std::u32string grapheme::NFC() const noexcept {
    return Foundation_globals->unicodeData->toNFC(static_cast<std::u32string>(*this));
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
