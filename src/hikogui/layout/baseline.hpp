// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
#include "hikogui/units/pixels.hpp"
#include <functional>
#include <utility>

hi_export_module(hikogui.text : baseline);

hi_export namespace hi::inline v1 {

enum class baseline_priority : unsigned int {
    none = 0,
    label = 1,
    small_widget = 10,
    large_widget = 100,
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
    struct baseline_function_result {
        float bottom;
        float middle;
        float top;
    };
    using baseline_function_type = std::function<baseline_function_result(float)>;

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
        _priority(baseline_priority::none), _function([](float height) {
            return baseline_function_result{0.0f, height / 2.0f, height};
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
     * @brief Creates a baseline object from the given cap height.
     *
     * This is for basic widgets where the text to be aligned is the only
     * object that will be rendered in the widget.
     *
     * @param priority The priority of the baseline.
     * @param cap_height The cap-height of the font in pixels.
     * @return The baseline object.
     */
    [[nodiscard]] static baseline from_cap_height(baseline_priority priority, float cap_height) noexcept
    {
        return hi::baseline{
            priority, [cap_height](float height) {
                return baseline_function_result{0.0f, height / 2.0f - cap_height / 2.0f, height - cap_height};
            }};
    }

    /**
     * Calculates the baseline from the middle of an object.
     * 
     * @param priority The priority of the baseline.
     * @param cap_height The cap height of the font in pixels.
     * @param object_height The height of the object in pixels.
     * @return The baseline.
     */
    [[nodiscard]] static baseline from_middle_of_object(baseline_priority priority, float cap_height, float object_height) noexcept
    {
        return hi::baseline{
            priority, [cap_height, object_height](float height) {
                return baseline_function_result{
                    object_height / 2.0f - cap_height / 2.0f,
                    height / 2.0f - cap_height / 2.0f,
                    height - object_height / 2.0f - cap_height / 2.0f};
            }};
    }

    /**
     * Creates a baseline object from the given cap height and padding values.
     *
     * @param priority The priority of the baseline.
     * @param cap_height The cap-height of the font in pixels.
     * @param bottom_padding The bottom padding of the box where the object is
     *                       aligned.
     * @param top_padding The top padding of the box where the object is
     *                    aligned.
     * @return The created baseline object.
     */
    [[nodiscard]] static baseline from_cap_height_and_padding(
        baseline_priority priority,
        float cap_height,
        float bottom_padding,
        float top_padding) noexcept
    {
        return hi::baseline{priority, [cap_height, bottom_padding, top_padding](float height) {
                                auto const padded_height = height - bottom_padding - top_padding;

                                return baseline_function_result{
                                    bottom_padding,
                                    bottom_padding + padded_height / 2.0f - cap_height / 2.0f,
                                    bottom_padding + padded_height - cap_height};
                            }};
    }

    /**
     * Embeds the given baseline function into a new baseline function with additional padding.
     *
     * This is used when a object with a baseline is embedded inside an object
     * which introduces padding around the object.
     *
     * @param priority The priority of the new baseline function.
     * @param other The baseline function to embed.
     * @param bottom_padding The amount of bottom padding to add.
     * @param top_padding The amount of top padding to add.
     * @return The new embedded baseline function.
     */
    [[nodiscard]] friend baseline
    embed(baseline const& other, baseline_priority priority, float bottom_padding, float top_padding) noexcept
    {
        return hi::baseline{
            priority, [embedded_func = other._function, bottom_padding, top_padding](float height) {
                auto const padded_height = height - bottom_padding - top_padding;
                auto const [bottom, middle, top] = embedded_func(padded_height);

                return baseline_function_result{bottom_padding + bottom, bottom_padding + middle, bottom_padding + top};
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
    [[nodiscard]] friend baseline lift(baseline const& other, float bottom_padding, float top_padding) noexcept
    {
        return hi::baseline{
            other._priority, [embedded_func = other._function, bottom_padding, top_padding](float height) {
                auto const padded_height = height + bottom_padding + top_padding;
                auto const [bottom, middle, top] = embedded_func(padded_height);

                return baseline_function_result{bottom - bottom_padding, middle - bottom_padding, top - bottom_padding};
            }};
    }

    [[nodiscard]] constexpr baseline_priority priority() const noexcept
    {
        return _priority;
    }

    /**
     * Calculates the baseline position based on the given height and alignment.
     *
     * @param height The height of the box in which an object must be aligned to
     *               the baseline.
     * @param alignment The vertical alignment of the object.
     * @return The baseline position from the bottom of the box.
     */
    [[nodiscard]] constexpr float get_baseline(float height, vertical_alignment alignment) const
    {
        assert(_function);

        auto const [bottom, middle, top] = _function(height);
        switch (alignment) {
        case vertical_alignment::none:
            std::unreachable();
        case vertical_alignment::top:
            return top;
        case vertical_alignment::bottom:
            return bottom;
        case vertical_alignment::middle:
            return middle;
        }
        std::unreachable();
    }

    /**
     * Calculates the middle position of an element based on its height, vertical alignment, and cap height.
     * 
     * @param height The height of the element.
     * @param alignment The vertical alignment of the element.
     * @param cap_height The cap height of the font of the element.
     * @return The middle position of text aligned to the @a alignment.
     */
    [[nodiscard]] constexpr float get_middle(float height, vertical_alignment alignment, float cap_height) const
    {
        return get_baseline(height, alignment) + cap_height / 2.0f;
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
