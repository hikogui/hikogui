// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"
#include "skeleton_parse_context.hpp"
#include "../resource_view.hpp"

namespace tt {

[[nodiscard]] std::unique_ptr<skeleton_node> parse_skeleton(skeleton_parse_context &context);

[[nodiscard]] inline std::unique_ptr<skeleton_node> parse_skeleton(URL url, std::string_view::const_iterator first, std::string_view::const_iterator last) {
    auto context = skeleton_parse_context(std::move(url), first, last);
    auto e = parse_skeleton(context);
    return e;
}

[[nodiscard]] inline std::unique_ptr<skeleton_node> parse_skeleton(URL url, std::string_view text)
{
    return parse_skeleton(std::move(url), text.cbegin(), text.cend());
}

[[nodiscard]] inline std::unique_ptr<skeleton_node> parse_skeleton(URL url)
{
    ttlet fv = url.loadView();
    ttlet sv = fv->string_view();

    return parse_skeleton(std::move(url), sv.cbegin(), sv.cend());
}

}
