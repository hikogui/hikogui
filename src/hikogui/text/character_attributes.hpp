
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

class character_attributes {
public:
    using value_type = uint64_t;

    constexpr character_attributes() noexcept = default;
    constexpr character_attributes(character_attributes const&) noexcept = default;
    constexpr character_attributes(character_attributes&&) noexcept = default;
    constexpr character_attributes& operator=(character_attributes const&) noexcept = default;
    constexpr character_attributes& operator=(character_attributes&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(character_attributes const&, character_attributes const&) noexcept = default;

    constexpr character_attributes(intrinsic_t, value_type value) noexcept : _value(value)
    {
        hi_axiom(_value <= 0x0000'07ff'ffff'ffffULL);
    }

    [[nodiscard]] constexpr value_type const& intrinsic() const
    {
        return _value;
    }

    [[nodiscard]] constexpr value_type & intrinsic()
    {
        return _value;
    }

    [[nodiscard]] constexpr hi::text_phrasing phrasing() const noexcept
    {
        return static_cast<hi::text_phrasing>((_value >> _phrasing_shift) & _phrasing_mask);
    }

    constexpr character_attributes& set_phrasing(hi::text_phrasing phrasing) noexcept
    {
        hilet phrasing_value = wide_cast<value_type>(to_underlying(phrasing));
        hi_axiom(phrasing_value <= _phrasing_mask);
        _value &= ~(_phrasing_mask << _phrasing_shift);
        _value |= phrasing_value << _phrasing_shift;
        return *this;
    }

    [[nodiscard]] constexpr iso_639 language() const noexcept
    {
        return iso_639{intrinsic_t{}, (_value >> _language_shift) & _language_mask};
    }

    constexpr character_attributes& set_language(iso_639 language) noexcept
    {
        hilet language_value = wide_cast<value_type>(language.intrinsic());
        hi_axiom(language_value <= _language_mask);
        _value &= ~(_language_mask << _language_shift);
        _value |= language_value << _language_shift;
        return *this;
    }

    [[nodiscard]] constexpr iso_3166 region() const noexcept
    {
        return iso_3166{intrinsic_t{}, (_value >> _region_shift) & _region_mask};
    }

    constexpr character_attributes& set_region(iso_3166 region) noexcept
    {
        hilet region_value = wide_cast<value_type>(region.intrinsic());
        hi_axiom(region_value <= _region_mask);
        _value &= ~(_region_mask << _region_shift);
        _value |= region_value << _region_shift;
        return *this;
    }

    constexpr character_attributes& set_language(hi::language_tag language_tag) noexcept
    {
        return set_language(language_tag.language).set_region(language_tag.region);
    }

    [[nodiscard]] constexpr text_theme theme() const noexcept
    {
        return text_theme{intrinsic_t{}, (_value >> _text_theme_shift) & _text_theme_mask};
    }

    constexpr character_attributes& set_theme(hi::text_theme text_theme) noexcept
    {
        hilet text_theme_value = wide_cast<value_type>(text_theme.intrinsic());
        hi_axiom(text_theme_value <= _text_theme_mask);
        _value &= ~(_text_theme_mask << _text_theme_shift);
        _value |= text_theme_value << _text_theme_shift;
        return *this;
    }

    template<character_attribute... Args>
    constexpr character_attributes(Args const&...args) noexcept
    {
        add(args...);
    }

    constexpr void add() noexcept {}

    constexpr void add(iso_639 const& arg) noexcept
    {
        set_language(arg);
    }

    constexpr void add(iso_3166 const& arg) noexcept
    {
        set_region(arg);
    }

    constexpr void add(text_phrasing const& arg) noexcept
    {
        set_phrasing(arg);
    }

    constexpr void add(text_theme const& arg) noexcept
    {
        set_theme(arg);
    }

    template<character_attribute First, character_attribute Second, character_attribute... Rest>
    constexpr void add(First const& first, Second const& second, Rest const&...rest) noexcept
    {
        add(first);
        add(second, rest...);
    }

private:
    constexpr static auto _text_theme_mask = uint64_t{0x1fff};
    constexpr static auto _text_theme_shift = 0U;
    constexpr static auto _phrasing_mask = uint64_t{0xf};
    constexpr static auto _phrasing_shift = 13U;
    constexpr static auto _region_mask = uint64_t{0x3ff};
    constexpr static auto _region_shift = 17U;
    constexpr static auto _language_mask = uint64_t{0xffff};
    constexpr static auto _language_shift = 27U;

    /** The value.
     *
     * [12:0] text-theme
     * [16:13] phrasing
     * [26:17] region.
     * [42:27] language
     */
    value_type _value;
};

}} // namespace hi::v1
