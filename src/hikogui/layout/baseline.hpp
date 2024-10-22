// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "../geometry/geometry.hpp"
#include "../units/units.hpp"
#include <functional>
#include <utility>

hi_export_module(hikogui.text : baseline);

hi_export namespace hi::inline v1 {
/**
 * @brief Enumeration representing the priority levels for baselines.
 *
 * The baseline_priority enumeration defines the priority levels for baselines.
 * Each priority level represents a different alignment preference for widgets
 * when negotiating the baseline position.
 */
enum class baseline_priority : unsigned int {
    none = 0, //< No priority.
    label = 1, //< Priority for labels.
    small_widget = 10, //< Priority for small widgets.
    large_widget = 100, //< Priority for large widgets.
};

/** The negotiated baseline between multiple objects with different alignments.
 *
 * This is used when multiple widgets are side by side, for example when they
 * are located in a row of a grid. Each widget will have the same height, but
 * the height will not yet be known when the negotiation starts.
 *
 * Once the negotiation is complete, the baseline will be calculated for each
 * widget based on the height of the row and the vertical alignment of the
 * widget.
 *
 * Since the height is unknown each widget will supply a function that will
 * calculate the baseline based on the height of the row. The function will be
 * called with the height of the row and will return the baseline position in
 * pixels from the bottom of the row.
 */
class baseline {
public:
    using baseline_function_type = std::function<unit::pixels_f(unit::pixels_f)>;

    baseline(baseline const&) noexcept = default;
    baseline(baseline&&) noexcept = default;
    baseline& operator=(baseline const&) noexcept = default;
    baseline& operator=(baseline&&) noexcept = default;

    /**
     * @brief Constructs a baseline object with default values.
     *
     * This constructor initializes the baseline object with a priority of 0 and a default lambda function.
     * The lambda function calculates the baseline position based on the given height.
     *
     * @note The default lambda function returns a `baseline_function_result` object with a baseline position of 0,
     *       a baseline offset of half the height, and the given height.
     */
    baseline() noexcept :
        _priority(baseline_priority::none), _function([](unit::pixels_f height) {
            return unit::pixels(0.0f);
        })
    {
    }

    /**
     * Create a baseline function that calculates the baseline position based on
     * the given height and alignment.
     *
     * @param priority The priority of the baseline.
     * @param function A function that calculates the baseline position based on
     *                 the given height. The function will return a tuple with
     *                 the baseline position for the bottom, middle and top of
     *                 the object. The arguments of the function us the height
     *                 of the box which contains the object to be aligned.
     */
    baseline(baseline_priority priority, baseline_function_type function) noexcept :
        _priority(priority), _function(std::move(function))
    {
    }

    /**
     * Calculates the baseline from the middle of an object.
     *
     * @param priority The priority of the baseline.
     * @param cap_height The cap height of the font in pixels.
     * @param object_height The height of the object in pixels.
     * @return The baseline.
     */
    [[nodiscard]] static baseline
    from_middle_of_object(baseline_priority priority, unit::pixels_f cap_height, unit::pixels_f object_height) noexcept
    {
        return hi::baseline{priority, [cap_height, object_height](unit::pixels_f height) {
                                return height / 2.0f - cap_height / 2.0f;
                            }};
    }

    /**
     * Embeds the given baseline function into a new baseline function with additional padding.
     *
     * This is used when a object with a baseline is embedded inside an object
     * which introduces padding around the object.
     *
     * @param other The baseline function to embed.
     * @param bottom_padding The amount of bottom padding to add.
     * @param top_padding The amount of top padding to add.
     * @return The new embedded baseline function.
     */
    [[nodiscard]] friend baseline embed(baseline const& other, unit::pixels_f bottom_padding, unit::pixels_f top_padding) noexcept
    {
        return hi::baseline{other._priority, [embedded_function = other._function, bottom_padding, top_padding](unit::pixels_f height) {
                                auto const unpadded_height = height - bottom_padding - top_padding;
                                return bottom_padding + embedded_function(unpadded_height);
                            }};
    }

    /**
     * Lifts the given baseline by applying bottom and top padding.
     *
     * This is used when an embedded object is layed out, and the padding
     * around the embedded object is removed.
     *
     * @param other The baseline to lift.
     * @param bottom_padding The amount of bottom padding to apply.
     * @param top_padding The amount of top padding to apply.
     * @return The lifted baseline.
     */
    [[nodiscard]] friend baseline lift(baseline const& other, unit::pixels_f bottom_padding, unit::pixels_f top_padding) noexcept
    {
        return hi::baseline{other._priority, [lifted_function = other._function, bottom_padding, top_padding](unit::pixels_f height) {
                                auto const padded_height = height + bottom_padding + top_padding;
                                return lifted_function(padded_height) - bottom_padding;
                            }};
    }

    [[nodiscard]] constexpr baseline_priority priority() const noexcept
    {
        return _priority;
    }

    /** Calculates the baseline position based on the given height.
     *
     * @param height The height of the box in which an object must be aligned to
     *               the baseline.
     * @return The baseline position from the bottom of the box.
     */
    [[nodiscard]] unit::pixels_f get_baseline(unit::pixels_f height) const
    {
        assert(_function);
        return _function(height);
    }

    /** Calculates the middle position of an element based on its height and cap-height.
     *
     * @param height The height of the element.
     * @param cap_height The cap height of the font of the element.
     * @return The middle position of text aligned to the @a alignment.
     */
    [[nodiscard]] unit::pixels_f get_middle(unit::pixels_f height, unit::pixels_f cap_height) const
    {
        return get_baseline(height) + cap_height / 2.0f;
    }

    /**
     * Returns the baseline object with the highest priority.
     *
     * @param lhs The first baseline object.
     * @param rhs The second baseline object.
     * @return The baseline object with the higher priority.
     */
    [[nodiscard]] friend baseline max(baseline const& a, baseline const& b) noexcept
    {
        if (a._priority > b._priority) {
            return a;
        } else {
            return b;
        }
    }

    template<std::convertible_to<baseline>... Rest>
    [[nodiscard]] friend baseline max(baseline const& a, baseline const& b, baseline const& c, Rest const&... rest) noexcept
    {
        return max(a, max(b, c, rest...));
    }

private:
    baseline_priority _priority = baseline_priority::none;
    baseline_function_type _function = {};
};
}
