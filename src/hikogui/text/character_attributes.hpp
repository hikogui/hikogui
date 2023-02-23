
#pragma once

#include "../i18n/language_tag.hpp"
#include "text_phrasing.hpp"

namespace hi { inline namespace v1 {

template<typename Context>
concept character_attribute = std::same_as<Context, iso_639> or std::same_as<Context, iso_3166> or
    std::same_as<Context, iso_15924> or std::same_as<Context, language_tag> or std::same_as<Context, text_phrasing>;

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
        // 40 bits.
        hi_axiom(_value < (value_type{1} << 40));
    }

    [[nodiscard]] constexpr value_type const& intrinsic() const
    {
        return _value;
    }

    [[nodiscard]] constexpr value_type& intrinsic()
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

    [[nodiscard]] constexpr iso_15924 script() const noexcept
    {
        return iso_15924{intrinsic_t{}, (_value >> _script_shift) & _script_mask};
    }

    constexpr character_attributes& set_script(iso_15924 script) noexcept
    {
        hilet script_value = wide_cast<value_type>(script.intrinsic());
        hi_axiom(script_value <= _script_mask);
        _value &= ~(_script_mask << _script_shift);
        _value |= script_value << _script_shift;
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
        return set_language(language_tag.language).set_script(language_tag.script).set_region(language_tag.region);
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

    constexpr void add(iso_15924 const& arg) noexcept
    {
        set_script(arg);
    }

    constexpr void add(iso_3166 const& arg) noexcept
    {
        set_region(arg);
    }

    constexpr void add(text_phrasing const& arg) noexcept
    {
        set_phrasing(arg);
    }

    template<character_attribute First, character_attribute Second, character_attribute... Rest>
    constexpr void add(First const& first, Second const& second, Rest const&...rest) noexcept
    {
        add(first);
        add(second, rest...);
    }

private:
    constexpr static auto _phrasing_mask = value_type{0xf};
    constexpr static auto _phrasing_shift = 36U;
    constexpr static auto _region_mask = value_type{0x3ff};
    constexpr static auto _region_shift = 26U;
    constexpr static auto _script_mask = value_type{0x3ff};
    constexpr static auto _script_shift = 16U;
    constexpr static auto _language_mask = value_type{0xffff};
    constexpr static auto _language_shift = 0U;

    /** The value.
     *
     * [15: 0] language
     * [25:16] script
     * [35:26] region
     * [39:36] phrasing
     */
    value_type _value;
};

}} // namespace hi::v1
