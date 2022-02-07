// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_db.hpp"
#include <exception>

namespace tt::inline v1 {

static unicode_description const &Replacement_Character_unicode_description = unicode_description::find(U'\ufffd');
static unicode_description const &CJK_Ideograph_Extension_A_unicode_description = unicode_description::find(U'\u3400');
static unicode_description const &CJK_Ideograph_unicode_description = unicode_description::find(U'\u4e00');
static unicode_description const &Hangul_Syllable_LV_unicode_description = unicode_description::find(U'\uac00');
static unicode_description const &Hangul_Syllable_LVT_unicode_description = unicode_description::find(U'\ud7a3');
static unicode_description const &Non_Private_Use_High_Surrogate_unicode_description =
    unicode_description::find(char32_t{0xd800});
static unicode_description const &Private_Use_High_Surrogate_unicode_description = unicode_description::find(char32_t{0xdb80});
static unicode_description const &Low_Surrogate_unicode_description = unicode_description::find(char32_t{0xdc00});
static unicode_description const &Private_Use_unicode_description = unicode_description::find(U'\ue000');
static unicode_description const &Tangut_Ideograph_unicode_description = unicode_description::find(U'\U00017000');
static unicode_description Extended_Pictographic_description =
    unicode_description::make_unassigned(unicode_description::find(U'\U0001f000'));
static unicode_description const &CJK_Ideograph_Extension_B_unicode_description = unicode_description::find(U'\U00020000');
static unicode_description const &CJK_Ideograph_Extension_C_unicode_description = unicode_description::find(U'\U0002a700');
static unicode_description const &CJK_Ideograph_Extension_D_unicode_description = unicode_description::find(U'\U0002b740');
static unicode_description const &CJK_Ideograph_Extension_E_unicode_description = unicode_description::find(U'\U0002b820');
static unicode_description const &CJK_Ideograph_Extension_F_unicode_description = unicode_description::find(U'\U0002ceb0');
static unicode_description const &Plane_15_Private_Use_unicode_description = unicode_description::find(U'\U000f0000');
static unicode_description const &Plane_16_Private_Use_unicode_description = unicode_description::find(U'\U00100000');

[[nodiscard]] unicode_description const &unicode_description::find(char32_t code_point) noexcept
{
    ttlet first = cbegin(detail::unicode_db_description_table);
    ttlet last = cend(detail::unicode_db_description_table);

    uint32_t code_point_ = static_cast<uint32_t>(code_point) << code_point_shift;
    auto it = std::lower_bound(first, last, code_point_, [](auto const &item, auto const &value) {
        return item._general_info < value;
    });

    if (it != last and *it == code_point) {
        return *it;

    } else {
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

        } else if (code_point >= U'\U0001f000' and code_point <= U'\U0001fffd') {
            return Extended_Pictographic_description;

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
    }
}

} // namespace tt::inline v1
