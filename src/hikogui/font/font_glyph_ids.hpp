// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "glyph_id.hpp"
#include "font_id.hpp"
#include "font_metrics.hpp"
#include "../container/container.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.font : font_glyphs_ids);

hi_export namespace hi { inline namespace v1 {

struct font_glyph_ids {
    using container_type = lean_vector<glyph_id>;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;

    hi::font_id font = {};
    container_type glyphs = {};

    font_glyph_ids(font_glyph_ids const&) noexcept = default;
    font_glyph_ids(font_glyph_ids &&) noexcept = default;
    font_glyph_ids& operator=(font_glyph_ids const&) noexcept = default;
    font_glyph_ids& operator=(font_glyph_ids &&) noexcept = default;
    constexpr font_glyph_ids() noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(font_glyph_ids const&, font_glyph_ids const&) noexcept = default;

    font_glyph_ids(hi::font_id font, lean_vector<glyph_id> glyphs) :
        font(font), glyphs(std::move(glyphs))
    {
        hi_axiom(not this->font.empty());
        hi_axiom(not this->glyphs.empty());
        for (auto glyph_id : this->glyphs) {
            hi_axiom(not glyph_id.empty());
        }
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return font.empty();
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] iterator begin() noexcept
    {
        return glyphs.begin();
    }

    [[nodiscard]] iterator end() noexcept
    {
        return glyphs.end();
    }

    [[nodiscard]] const_iterator begin() const noexcept
    {
        return glyphs.begin();
    }

    [[nodiscard]] const_iterator end() const noexcept
    {
        return glyphs.end();
    }

    [[nodiscard]] const_iterator cbegin() const noexcept
    {
        return glyphs.cbegin();
    }

    [[nodiscard]] const_iterator cend() const noexcept
    {
        return glyphs.cend();
    }

    [[nodiscard]] glyph_id front() const
    {
        hi_axiom(not glyphs.empty());
        return glyphs.front();
    }

    [[nodiscard]] glyph_id back() const
    {
        hi_axiom(not glyphs.empty());
        return glyphs.back();
    }

    [[nodiscard]] glyph_id operator[](size_t i) const
    {
        return glyphs[i];
    }

    [[nodiscard]] font_metrics_em const& font_metrics() const
    {
        hi_axiom(not font.empty());
        return font->metrics;
    }

    [[nodiscard]] hi::glyph_metrics glyph_metrics(size_t i) const
    {
        hi_axiom(i < glyphs.size());
        hi_axiom(not font.empty());
        return font->get_metrics(glyphs[i]);
    }

    [[nodiscard]] hi::glyph_metrics front_glyph_metrics() const
    {
        return glyph_metrics(0);
    }
};


}}
