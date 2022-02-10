// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "grapheme.hpp"
#include "unicode_normalization.hpp"
#include "unicode_description.hpp"

namespace tt::inline v1 {

[[nodiscard]] static grapheme::value_type make_grapheme(std::u32string_view code_points) noexcept
{
    uint64_t value = 0;

    if (not code_points.empty()) {
        // Set the starter code-point.
        auto it = code_points.begin();
        value = static_cast<grapheme::value_type>(*it++) << 43;

        // Set the length.
        value |= static_cast<grapheme::value_type>(code_points.size() <= 5 ? code_points.size() : 6);

        // Add the non-starter code-points.
        auto i = 1_uz;
        for (; i != 5 and it != code_points.end(); ++i, ++it) {
            ttlet &description = unicode_description::find(*it);
            ttlet shift = (4 - i) * 10 + 3;
            value |= static_cast<grapheme::value_type>(description.non_starter_code()) << shift;
        }
    }

    return value;
}

grapheme::grapheme(std::u32string_view code_points) noexcept : value(make_grapheme(unicode_NFKC(code_points))) {
}

grapheme &grapheme::operator=(std::u32string_view code_points) noexcept
{
    value = make_grapheme(unicode_NFKC(code_points));
    return *this;
}

[[nodiscard]] grapheme grapheme::from_composed(std::u32string_view code_points) noexcept
{
    grapheme r;
    r.value = make_grapheme(code_points);
    return r;
}

[[nodiscard]] std::u32string grapheme::decomposed() const noexcept
{
    return unicode_NFD(composed());
}

[[nodiscard]] bool grapheme::valid() const noexcept
{
    if (empty()) {
        return false;
    }

    if (is_noncharacter(get<0>(*this))) {
        return false;
    }

    ttlet &description = unicode_description::find(get<0>(*this));
    if (is_C(description)) {
        return false;
    }
    if (description.canonical_combining_class() != 0) {
        return false;
    }
    return true;
}

} // namespace tt::inline v1
