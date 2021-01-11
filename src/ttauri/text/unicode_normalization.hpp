// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <string>
#include <string_view>

namespace tt {

/** Convert text to Unicode-NFD normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param ligatures typographical-ligatures such as "fi" are decomposed.
 * @param paragraph line-feed characters are converted to paragraph separators.
 */
std::u32string unicode_NFD(std::u32string_view text, bool ligatures = false, bool paragraph = false) noexcept;

/** Convert text to Unicode-NFC normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param ligatures typographical-ligatures such as "fi" are decomposed.
 * @param paragraph line-feed characters are converted to paragraph separators.
 * @param composeCRLF Compose CR-LF combinations to LF.
 */
[[nodiscard]] std::u32string
unicode_NFC(std::u32string_view text, bool ligatures = false, bool paragraph = false, bool composeCRLF = false)
    noexcept;

/** Convert text to Unicode-NFKD normal form.
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param paragraph line-feed characters are converted to paragraph separators.
 */
std::u32string unicode_NFKD(std::u32string_view text, bool paragraph = false) noexcept;

/** Convert text to Unicode-NFKC normal form.
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param paragraph line-feed characters are converted to paragraph separators.
 * @param composeCRLF Compose CR-LF combinations to LF.
 */
std::u32string unicode_NFKC(std::u32string_view text, bool paragraph = false, bool composeCRLF = false) noexcept;

}
