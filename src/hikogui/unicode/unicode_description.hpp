// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_script.hpp"
#include "../utility/module.hpp"

namespace hi::inline v1 {

constexpr char32_t unicode_replacement_character = U'\ufffd';
constexpr char32_t unicode_LF = U'\n';
constexpr char32_t unicode_VT = U'\v';
constexpr char32_t unicode_FF = U'\f';
constexpr char32_t unicode_CR = U'\r';
constexpr char32_t unicode_NEL = U'\u0085';
constexpr char32_t unicode_LS = U'\u2028';
constexpr char32_t unicode_PS = U'\u2029';

/** Description of a unicode code point.
 * This class holds information of a unicode code point.
 *
 * The information is compressed to use bit-fields to reduce memory usage
 * of the unicode database.
 */
class unicode_description {
public:
    constexpr unicode_description() noexcept = default;
    unicode_description(unicode_description const&) = delete;
    unicode_description& operator=(unicode_description const&) = delete;
    constexpr unicode_description(unicode_description&&) noexcept = default;
    constexpr unicode_description& operator=(unicode_description&&) noexcept = default;

    [[nodiscard]] constexpr unicode_description(
        unicode_script script) noexcept :
        _script(to_underlying(script))
    {
        hi_assert(to_underlying(script) <= 0xff);
    }

    /** Get the script of this character.
     */
    [[nodiscard]] constexpr unicode_script script() const noexcept
    {
        return static_cast<unicode_script>(_script);
    }

    /** Find a code-point in the global unicode_description table.
     * For any valid unicode code point this function will return a reference to
     * the unicode_description. It may return a unicode_description to the
     * U+fffd 'REPLACEMENT CHARACTER' if the code-point could not be found in the
     * table. Or it may return unicode_description to a single element in a range
     * of code-points, such as for hangul-syllables, or private use areas.
     *
     * @param code_point The code point to look up.
     * @return a const reference to the unicode_description entry.
     */
    [[nodiscard]] static unicode_description const& find(char32_t code_point) noexcept;

private:
    // 1st qword
    uint8_t _script : 8;
};

static_assert(sizeof(unicode_description) == 1);

} // namespace hi::inline v1
