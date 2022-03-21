// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <optional>
#include <string_view>

namespace tt::inline v1 {

/** Expand a short language tag to the most likely long language tag.
 */
[[nodiscard]] std::optional<std::string_view> expand_language_tag(std::string_view from) noexcept;

} // namespace tt::inline v1