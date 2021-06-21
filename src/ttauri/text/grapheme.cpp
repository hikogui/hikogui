// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grapheme.hpp"
#include "unicode_normalization.hpp"

namespace tt {

grapheme::grapheme(std::u32string_view codePoints) noexcept :
    value(0)
{
    ttlet codePoints_ = unicode_NFC(codePoints);

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

grapheme& grapheme::operator+=(char32_t codePoint) noexcept
{
    tt_axiom(size() < std::tuple_size_v<long_grapheme>);
    switch (size()) {
    case 0:
        value |= (static_cast<uint64_t>(codePoint & 0x1f'ffff) << 1);
        break;
    case 1:
        value |= (static_cast<uint64_t>(codePoint & 0x1f'ffff) << 22);
        break;
    case 2:
        value |= (static_cast<uint64_t>(codePoint & 0x1f'ffff) << 43);
        break;
    case 3: {
        std::array<char32_t,4> tmp;
        tmp[0] = (*this)[0];
        tmp[1] = (*this)[1];
        tmp[2] = (*this)[2];
        tmp[3] = codePoint;
        value = create_pointer(tmp.data(), 3);
        } break;
    default:
        auto tmp = *get_pointer();
        tmp[size()] = codePoint;
    }
    return *this;
}

[[nodiscard]] std::u32string grapheme::NFD() const noexcept {
    return unicode_NFD(static_cast<std::u32string>(*this));
}

[[nodiscard]] std::u32string grapheme::NFKC() const noexcept {
    return unicode_NFKC(static_cast<std::u32string>(*this));
}

[[nodiscard]] std::u32string grapheme::NFKD() const noexcept {
    return unicode_NFKD(static_cast<std::u32string>(*this));
}


}
