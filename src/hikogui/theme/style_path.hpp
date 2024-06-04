// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include <string>
#include <vector>
#include <cstdint>

hi_export_module(hikogui.theme : style_path);

hi_export namespace hi {
inline namespace v1 {

struct style_path_segment {
    std::string name;
    std::string id;
    std::vector<std::string> classes;

    constexpr style_path_segment() noexcept = default;
    constexpr style_path_segment(style_path_segment const&) = default;
    constexpr style_path_segment(style_path_segment&&) = default;
    constexpr style_path_segment& operator=(style_path_segment const&) = default;
    constexpr style_path_segment& operator=(style_path_segment&&) = default;
    [[nodiscard]] constexpr friend bool operator==(style_path_segment const&, style_path_segment const&) noexcept = default;

    constexpr style_path_segment(std::string name, std::string id = {}, std::vector<std::string> classes = {}) :
        name(std::move(name)), id(std::move(id)), classes(std::move(classes)) {}
};

class style_path : public std::vector<style_path_segment> {
    using super = std::vector<style_path_segment>;
    using super::super;
};

}}
