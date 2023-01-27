// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ucd_index.hpp"
#include "ucd_descriptions.hpp"
#include "ucd_compositions.hpp"
#include "ucd_decompositions.hpp"
#include "unicode_description.hpp"
#include "../utility/module.hpp"

namespace hi::inline v1 {

[[nodiscard]] unicode_description const& unicode_description::find(char32_t code_point) noexcept
{
    hi_assert(code_point <= 0x10'ffff);

    auto index = wide_cast<size_t>(ucd_index[code_point >> 5]);
    index <<= 5;
    index |= code_point & 0x1f;
    return ucd_descriptions[index];
}

[[nodiscard]] std::u32string unicode_description::decompose() const noexcept
{
    constexpr uint64_t mask = 0x1f'ffff;
    constexpr char32_t terminator = 0x1f'ffff;

    auto r = std::u32string{};
    if (_decomposition_index <= 0x10'ffff) {
        r += truncate<char32_t>(_decomposition_index);
        return r;

    } else if (_decomposition_index < 0x1f'ffff) {
        auto index = _decomposition_index - 0x11'0000;

        uint64_t tmp = 0;
        do {
            hi_assert_bounds(index, ucd_decompositions);
            tmp = ucd_decompositions[index++];

            // Entry 0 [20:0]
            // Entry 1 [41:21]
            // Entry 2 [62:42]
            for (auto i = 0_uz; i != 3; ++i) {
                auto c = truncate<char32_t>(tmp & mask);
                if (c == terminator) {
                    return r;
                }

                r += c;
                tmp >>= 21;
            }

            // Continue if 0[63] == '0'.
        } while (tmp == 0);

        return r;
    } else {
        hi_no_default();
    }
}

[[nodiscard]] char32_t unicode_description::compose(char32_t other) const noexcept
{
    constexpr uint64_t mask = 0x1f'ffff;

    if (_composition_index == 0) {
        return 0xffff;
    }

    auto index = _composition_index - 1;

    uint64_t tmp = 0;
    do {
        // Entry 0[20:0] and 0[41:21], 0[63] == '-'.
        hi_assert_bounds(index, ucd_compositions);
        tmp = ucd_compositions[index++];
        if (hilet c = truncate<char32_t>(tmp & mask); c <= other) {
            tmp >>= 21;
            if (c == other) {
                return truncate<char32_t>(tmp & mask);
            }
        } else {
            return 0xffff;
        }
        tmp >>= 21;

        // Entry 0[62:42] and 1[20:0], 0[63] == '-'.
        if (hilet c = truncate<char32_t>(tmp & mask); c <= other) {
            hi_assert_bounds(index, ucd_compositions);
            tmp = ucd_compositions[index++];
            if (c == other) {
                return truncate<char32_t>(tmp & mask);
            }
        } else {
            return 0xffff;
        }
        tmp >>= 21;

        // Entry 1[41:21] and 1[62:42]
        if (hilet c = truncate<char32_t>(tmp & mask); c <= other) {
            tmp >>= 21;
            if (c == other) {
                return truncate<char32_t>(tmp & mask);
            }
        } else {
            return 0xffff;
        }
        tmp >>= 21;

        // Continue if 1[63] == '0'.
    } while (tmp == 0);

    return 0xffff;
}

} // namespace hi::inline v1
