

#pragma once

#include "../units/units.hpp"
#include "../color/color.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.theme : theme_attributes);

hi_export namespace hi {
inline namespace v1 {

template<typename T>
class theme_attribute {
public:
    using value_type = T;

    constexpr theme_attribute() noexcept = default;
    constexpr theme_attribute(theme_attribute const&) noexcept = default;
    constexpr theme_attribute(theme_attribute&&) noexcept = default;
    constexpr theme_attribute& operator==(theme_attribute const&) noexcept = default;
    constexpr theme_attribute& operator==(theme_attribute&&) noexcept = default;

    theme_attribute(std::nullopt_t) noexcept : _value(std::nullopt), important(false) {}

    template<std::convertible_to<value_type> Value>
    theme_attribute(Value&& value, bool important = false) noexcept : _value(std::forward<Value>(value)), important(important)
    {
    }

    theme_attribute& operator=(std::nullopt_t) noexcept
    {
        reset();
        return *this;
    }

    template<std::convertible_to<value_type> Value>
    theme_attribute& operator=(Value&& value) noexcept
    {
        set_value(std::forward<Value>(value));
        return *this;
    }

    [[nodiscard]] bool has_value() const noexcept
    {
        return _value.has_value();
    }

    [[nodiscard]] bool is_important() const noexcept
    {
        return _important;
    }

    explicit operator bool() const noexcept
    {
        return has_value();
    }

    [[nodiscard]] value_type operator*() const noexcept
    {
        hi_assert(value.has_value());
        return *_value;
    }

    void reset() noexcept
    {
        _value = std::nullopt;
        _important = false;
    }

    template<std::convertible_to<value_type> Value>
    void set_value(Value&& value, bool important = false) noexcept
    {
        if (not _important) {
            _value = std::forward<Value>(value);
            _important = important;
        }
    }

private:
    std::optional<value_type> _value = std::nullopt;
    bool _important = false;
};

struct theme_attributes {
    theme_attribute<length_f> width;
    theme_attribute<length_f> height;
    theme_attribute<length_f> margin_left;
    theme_attribute<length_f> margin_bottom;
    theme_attribute<length_f> margin_right;
    theme_attribute<length_f> margin_top;
    theme_attribute<length_f> padding_left;
    theme_attribute<length_f> padding_bottom;
    theme_attribute<length_f> padding_right;
    theme_attribute<length_f> padding_top;
    theme_attribute<length_f> border_width;
    theme_attribute<length_f> left_bottom_corner_radius;
    theme_attribute<length_f> right_bottom_corner_radius;
    theme_attribute<length_f> left_top_corner_radius;
    theme_attribute<length_f> right_top_corner_radius;

    theme_attribute<color> background_color;
    theme_attribute<color> foreground_color;
    theme_attribute<color> border_color;
};

} // namespace v1
}
