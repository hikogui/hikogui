
#pragma once

#include "../i18n/iso_639.hpp"
#include "../i18n/iso_3166.hpp"
#include "../i18n/language_tag.hpp"
#include "text_phrasing.hpp"
#include "text_theme.hpp"

namespace hi { inline namespace v1 {

template<typename Context>
concept character_attribute = std::same_as<Context, iso_639> or std::same_as<Context, iso_3166> or
    std::same_as<Context, language_tag> or std::same_as<Context, text_phrasing> or std::same_as<Context, text_theme>;

struct character_attributes {
    iso_639 language;
    iso_3166 country;
    text_phrasing phrasing;
    text_theme theme;

    template<character_attribute... Args>
    character_attributes(Args const &...args) noexcept
    {
        add(args...);
    }

    void add() noexcept {}

    void add(iso_639 const& arg) noexcept
    {
        language = arg;
    }

    void add(iso_3166 const& arg) noexcept
    {
        country = arg;
    }

    void add(text_phrasing const& arg) noexcept
    {
        prasing = arg;
    }

    void add(text_theme const& arg) noexcept
    {
        theme = arg;
    }

    template<character_attribute First, character_attribute Second, character_attribute... Rest>
    void add(First const &first, Second const &second, Rest const &...rest) noexcept
    {
        add(first);
        add(second, rest...);
    }
};

}} // namespace hi::v1