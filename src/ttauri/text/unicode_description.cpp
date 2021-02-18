// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/text/unicode_db.hpp"
#include <exception>

namespace tt {

constexpr auto _unicode_description_find(char32_t code_point) noexcept
{
    tt_axiom(code_point <= 0x10'ffff);

    auto first = std::begin(detail::unicode_db_description_table);
    auto last = std::end(detail::unicode_db_description_table);
    return unicode_description_find(first, last, code_point);
}

constexpr unicode_description const &Replacement_Character_unicode_description = *_unicode_description_find(U'\ufffd');
constexpr unicode_description const &CJK_Ideograph_Extension_A_unicode_description = *_unicode_description_find(U'\u3400');
constexpr unicode_description const &CJK_Ideograph_unicode_description = *_unicode_description_find(U'\u4e00');
constexpr unicode_description const &Hangul_Syllable_LV_unicode_description = *_unicode_description_find(U'\uac00');
constexpr unicode_description const &Hangul_Syllable_LVT_unicode_description = *_unicode_description_find(U'\ud7a3');
constexpr unicode_description const &Non_Private_Use_High_Surrogate_unicode_description =
    *_unicode_description_find(char32_t{0xd800});
constexpr unicode_description const &Private_Use_High_Surrogate_unicode_description =
    *_unicode_description_find(char32_t{0xdb80});
constexpr unicode_description const &Low_Surrogate_unicode_description = *_unicode_description_find(char32_t{0xdc00});
constexpr unicode_description const &Private_Use_unicode_description = *_unicode_description_find(U'\ue000');
constexpr unicode_description const &Tangut_Ideograph_unicode_description = *_unicode_description_find(U'\U00017000');
constexpr unicode_description const &CJK_Ideograph_Extension_B_unicode_description = *_unicode_description_find(U'\U00020000');
constexpr unicode_description const &CJK_Ideograph_Extension_C_unicode_description = *_unicode_description_find(U'\U0002a700');
constexpr unicode_description const &CJK_Ideograph_Extension_D_unicode_description = *_unicode_description_find(U'\U0002b740');
constexpr unicode_description const &CJK_Ideograph_Extension_E_unicode_description = *_unicode_description_find(U'\U0002b820');
constexpr unicode_description const &CJK_Ideograph_Extension_F_unicode_description = *_unicode_description_find(U'\U0002ceb0');
constexpr unicode_description const &Plane_15_Private_Use_unicode_description = *_unicode_description_find(U'\U000f0000');
constexpr unicode_description const &Plane_16_Private_Use_unicode_description = *_unicode_description_find(U'\U00100000');

[[nodiscard]] unicode_description const &unicode_description_find(char32_t code_point) noexcept
{
    auto last = std::end(detail::unicode_db_description_table);
    auto it = _unicode_description_find(code_point);

    if (it == last) {
        if (code_point >= U'\u3400' && code_point <= U'\u4db5') {
            return CJK_Ideograph_Extension_A_unicode_description;

        } else if (code_point >= U'\u4e00' && code_point <= U'\u9fef') {
            return CJK_Ideograph_unicode_description;

        } else if (code_point >= U'\uac00' && code_point <= U'\ud7a3') {
            if (is_hangul_LV_part(code_point)) {
                return Hangul_Syllable_LV_unicode_description;
            } else if (is_hangul_LVT_part(code_point)) {
                return Hangul_Syllable_LVT_unicode_description;
            } else {
                return Replacement_Character_unicode_description;
            }

        } else if (code_point >= char32_t{0xd800} && code_point <= char32_t{0xdb7f}) {
            return Non_Private_Use_High_Surrogate_unicode_description;

        } else if (code_point >= char32_t{0xdb80} && code_point <= char32_t{0xdbff}) {
            return Private_Use_High_Surrogate_unicode_description;

        } else if (code_point >= char32_t{0xdc00} && code_point <= char32_t{0xdfff}) {
            return Low_Surrogate_unicode_description;

        } else if (code_point >= U'\ue000' && code_point <= U'\uf8ff') {
            return Private_Use_unicode_description;

        } else if (code_point >= U'\U00017000' && code_point <= U'\U000187f7') {
            return Tangut_Ideograph_unicode_description;

        } else if (code_point >= U'\U00020000' && code_point <= U'\U0002a6d6') {
            return CJK_Ideograph_Extension_B_unicode_description;

        } else if (code_point >= U'\U0002a700' && code_point <= U'\U0002b734') {
            return CJK_Ideograph_Extension_C_unicode_description;

        } else if (code_point >= U'\U0002b740' && code_point <= U'\U0002b81d') {
            return CJK_Ideograph_Extension_D_unicode_description;

        } else if (code_point >= U'\U0002b820' && code_point <= U'\U0002cea1') {
            return CJK_Ideograph_Extension_E_unicode_description;

        } else if (code_point >= U'\U0002ceb0' && code_point <= U'\U0002ebe0') {
            return CJK_Ideograph_Extension_F_unicode_description;

        } else if (code_point >= U'\U000f0000' && code_point <= U'\U000ffffd') {
            return Plane_15_Private_Use_unicode_description;

        } else if (code_point >= U'\U00100000' && code_point <= U'\U0010fffd') {
            return Plane_16_Private_Use_unicode_description;

        } else {
            return Replacement_Character_unicode_description;
        }
    } else {
        return *it;
    }
}





} // namespace tt
