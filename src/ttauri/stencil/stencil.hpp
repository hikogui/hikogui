// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "stencil_node.hpp"
#include "stencil_parse_context.hpp"
#include "../ResourceView.hpp"

namespace tt {

[[nodiscard]] std::unique_ptr<stencil_node> parse_stencil(stencil_parse_context &context);

[[nodiscard]] inline std::unique_ptr<stencil_node> parse_stencil(URL url, std::string_view::const_iterator first, std::string_view::const_iterator last) {
    auto context = stencil_parse_context(std::move(url), first, last);
    auto e = parse_stencil(context);
    return e;
}

[[nodiscard]] inline std::unique_ptr<stencil_node> parse_stencil(URL url, std::string_view text) {
    return parse_stencil(std::move(url), text.cbegin(), text.cend());
}

[[nodiscard]] inline std::unique_ptr<stencil_node> parse_stencil(URL url) {
    ttlet fv = url.loadView();
    ttlet sv = fv->string_view();

    return parse_stencil(std::move(url), sv.cbegin(), sv.cend());
}

}
