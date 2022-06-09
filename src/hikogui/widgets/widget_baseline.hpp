// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

/** The base-line of a widget on which to set the text and graphics.
 */
class widget_baseline {
public:
    constexpr widget_baseline() noexcept = default;
    constexpr widget_baseline(widget_baseline const&) noexcept = default;
    constexpr widget_baseline(widget_baseline&&) noexcept = default;
    constexpr widget_baseline& operator=(widget_baseline const&) noexcept = default;
    constexpr widget_baseline& operator=(widget_baseline&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(widget_baseline const&, widget_baseline const&) noexcept = default;

    /** Construct a widget base-line.
     *
     * @param priority How sure a widget is that its base-line should be used.
     *                 0.0: bad, 0.1: text label, 0.5 small widget, 1.0 large widget.
     * @param gain The relative position of the base-line compared to the height of the widget.
     * @param bias The absolute offset of the base-line.
     */
    constexpr widget_baseline(float priority, float gain, float bias) noexcept : _priority(priority), _gain(gain), _bias(bias) {}

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _priority == 0.0f;
    }

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Calculate the absolute base-line.
     *
     * @param height The height of the widget.
     * @return height * gain + bias.
     */
    [[nodiscard]] constexpr float absolute(float height) const noexcept
    {
        return height * _gain + _bias;
    }

    /** Get the base-line of highest priority.
     */
    [[nodiscard]] constexpr friend widget_baseline max(widget_baseline const& lhs, widget_baseline const& rhs) noexcept
    {
        return lhs._priority > rhs._priority ? lhs : rhs;
    }

private:
    float _priority = 0.0f;
    float _gain = 0.0f;
    float _bias = 0.0f;
};

} // namespace hi::inline v1
