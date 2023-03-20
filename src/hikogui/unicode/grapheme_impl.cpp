// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grapheme.hpp"
#include "unicode_normalization.hpp"
#include "unicode_description.hpp"
#include "unicode_general_category.hpp"
#include "../log.hpp"
#include "../utility/module.hpp"
#include <mutex>

namespace hi::inline v1 {

grapheme::grapheme(composed_t, std::u32string_view code_points) noexcept
{
    switch (code_points.size()) {
    case 0:
        hi_no_default();

    case 1:
        _value = truncate<value_type>(code_points[0]);
        break;

    default:
        hilet index = detail::long_graphemes.insert(std::u32string{code_points});
        if (index < 0x0f'0000) {
            _value = narrow_cast<value_type>(index + 0x11'0000);
        } else {
            [[unlikely]] hi_log_error_once("grapheme::error::too-many", "Too many long graphemes encoded, replacing with U+fffd");
            _value = 0x00'fffd;
        }
    }
}

grapheme::grapheme(std::u32string_view code_points) noexcept :
    grapheme(composed_t{}, unicode_normalize(code_points, unicode_normalize_config::NFC()))
{
}

[[nodiscard]] std::u32string grapheme::decomposed() const noexcept
{
    return unicode_decompose(composed(), unicode_normalize_config::NFD());
}

[[nodiscard]] bool grapheme::valid() const noexcept
{
    if (is_noncharacter(get<0>(*this))) {
        return false;
    }

    hilet general_category = ucd_get_general_category(get<0>(*this));
    if (is_C(general_category)) {
        return false;
    }
    if (is_M(general_category)) {
        return false;
    }
    return true;
}

} // namespace hi::inline v1
