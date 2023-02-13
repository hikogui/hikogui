// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file pattern_match.hpp Defines the simple pattern match function.
 */

#pragma once

namespace hi { inline namespace v1 {

/** Patern match.
 *
 * @tparam Wildcard The wildcard symbol to use.
 * @param needle The string to match with haystack, may include wildcard symbols.
 * @param haystack The string to match with needle.
 * @return True when the @a needle pattern matched @a haystack.
 */
template<char Wildcard = '*'>
[[nodiscard]] constexpr bool pattern_match(std::string_view needle, std::string_view haystack) noexcept
{
    auto needle_index = 0_uz;
    auto haystack_index = 0_uz;

    while (needle_index != needle.size()) {
        // Find the next glob.
        hilet i = needle.find(Wildcard, needle_index);
        hilet pattern = needle.substr(needle_index, i);
        needle_index = i != needle.npos ? i + 1 : needle.size();

        // Check if the patterns between two globs is found in the haystack in-sequence.
        if (not pattern.empty()) {
            haystack_index = haystack.find(pattern, haystack_index);
            if (haystack_index == haystack.npos) {
                return false;
            }
            haystack_index += pattern.size();
        }
    }

    if (needle.empty() or needle.back() != Wildcard) {
        return haystack_index == haystack.size();
    }
    return true;
}

}} // namespace hi::v1
