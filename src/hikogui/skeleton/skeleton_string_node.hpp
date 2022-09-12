// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_node.hpp"

namespace hi::inline v1 {

struct skeleton_string_node final : skeleton_node {
    std::string text;

    skeleton_string_node(parse_location location, std::string text) : skeleton_node(std::move(location)), text(std::move(text)) {}

    [[nodiscard]] bool should_left_align() const noexcept override
    {
        return false;
    }

    void left_align() noexcept override
    {
        // first check if there are only spaces and tabs after the last line feed.
        auto new_text_length = ssize(text);
        for (auto i = text.crbegin(); i != text.crend(); ++i, --new_text_length) {
            if (*i == ' ' || *i == '\t') {
                // Strip spaces and tabs.
                continue;
            } else if (*i == '\n') {
                // Stop here, we are stripping a line upto the last line feed.
                break;
            } else {
                // If there are characters after the last line feed, we do not want to strip the line.
                return;
            }
        }
        text.resize(new_text_length);
    }

    std::string string() const noexcept override
    {
        return std::format("<text {}>", text);
    }

    datum evaluate(formula_evaluation_context &context) override
    {
        context.write(text);
        return {};
    }
};

} // namespace hi::inline v1