// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"
#include "skeleton_parse_context.hpp"
#include "../file/file_view.hpp"

namespace hi::inline v1 {

[[nodiscard]] std::unique_ptr<skeleton_node> parse_skeleton(skeleton_parse_context &context);

[[nodiscard]] inline std::unique_ptr<skeleton_node>
parse_skeleton(std::filesystem::path path, std::string_view::const_iterator first, std::string_view::const_iterator last)
{
    auto context = skeleton_parse_context(std::move(path), first, last);
    auto e = parse_skeleton(context);
    return e;
}

[[nodiscard]] inline std::unique_ptr<skeleton_node> parse_skeleton(std::filesystem::path path, std::string_view text)
{
    return parse_skeleton(std::move(path), text.cbegin(), text.cend());
}

[[nodiscard]] inline std::unique_ptr<skeleton_node> parse_skeleton(std::filesystem::path path)
{
    hilet fv = file_view(path);
    hilet sv = as_string_view(fv);

    return parse_skeleton(std::move(path), sv.cbegin(), sv.cend());
}

} // namespace hi::inline v1
