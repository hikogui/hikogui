// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gstring.hpp"
#include "unicode_grapheme_cluster_break.hpp"
#include "unicode_normalization.hpp"
#include "../strings.hpp"

namespace hi::inline v1 {

[[nodiscard]] gstring to_gstring(std::u32string_view rhs, unicode_normalize_config config) noexcept
{
    hilet normalized_string = unicode_normalize(rhs, config);

    auto r = gstring{};
    auto break_state = detail::grapheme_break_state{};
    auto cluster = std::u32string{};

    for (hilet code_point : normalized_string) {
        if (detail::breaks_grapheme(code_point, break_state)) {
            if (cluster.size() > 0) {
                r += grapheme(composed_t{}, cluster);
                hi_axiom(r.back().valid());
            }
            cluster.clear();
        }

        cluster += code_point;
    }
    if (ssize(cluster) != 0) {
        r += grapheme(composed_t{}, cluster);
        hi_assert(r.back().valid());
    }
    return r;
}

} // namespace hi::inline v1
