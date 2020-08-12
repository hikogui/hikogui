// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "veer_node.hpp"
#include "veer_parse_context.hpp"
#include "../ResourceView.hpp"

namespace tt {

[[nodiscard]] std::unique_ptr<veer_node> parse_veer(veer_parse_context &context);

[[nodiscard]] inline std::unique_ptr<veer_node> parse_veer(URL url, std::string_view::const_iterator first, std::string_view::const_iterator last) {
    auto context = veer_parse_context(std::move(url), first, last);
    auto e = parse_veer(context);
    return e;
}

[[nodiscard]] inline std::unique_ptr<veer_node> parse_veer(URL url, std::string_view text) {
    return parse_veer(std::move(url), text.cbegin(), text.cend());
}

[[nodiscard]] inline std::unique_ptr<veer_node> parse_veer(URL url) {
    ttlet fv = url.loadView();
    ttlet sv = fv->string_view();

    return parse_veer(std::move(url), sv.cbegin(), sv.cend());
}

}
