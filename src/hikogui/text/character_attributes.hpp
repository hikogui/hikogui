
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
    iso_639 language = {};
    iso_3166 region = {};
    text_phrasing phrasing = {};
    text_theme theme = {};

    constexpr character_attributes() noexcept = default;
    constexpr character_attributes(character_attributes const&) noexcept = default;
    constexpr character_attributes(character_attributes&&) noexcept = default;
    constexpr character_attributes& operator=(character_attributes const&) noexcept = default;
    constexpr character_attributes& operator=(character_attributes&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(character_attributes const&, character_attributes const&) noexcept = default;

    template<character_attribute... Args>
    constexpr character_attributes(Args const&...args) noexcept
    {
        add(args...);
    }

    constexpr void add() noexcept {}

    constexpr void add(iso_639 const& arg) noexcept
    {
        language = arg;
    }

    constexpr void add(iso_3166 const& arg) noexcept
    {
        region = arg;
    }

    constexpr void add(text_phrasing const& arg) noexcept
    {
        phrasing = arg;
    }

    constexpr void add(text_theme const& arg) noexcept
    {
        theme = arg;
    }

    template<character_attribute First, character_attribute Second, character_attribute... Rest>
    constexpr void add(First const& first, Second const& second, Rest const&...rest) noexcept
    {
        add(first);
        add(second, rest...);
    }
};

}} // namespace hi::v1