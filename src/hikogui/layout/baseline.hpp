// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../units/units.hpp"
#include "../macros.hpp"
#include "hikogui/units/pixels.hpp"
#include <functional>
#include <utility>

hi_export_module(hikogui.text : baseline);

hi_export namespace hi::inline v1 {
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
    using baseline_function_result = std::tuple<unit::pixels_f, unit::pixels_f, unit::pixels_f>;
    using baseline_function_type = std::function<baseline_function_result(unit::pixels_f)>;

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
        _priority(0), _function([](unit::pixels_f height) {
            return baseline_function_result{unit::pixels(0.0f), height / 2.0f, height};
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
    baseline(unsigned int priority, baseline_function_type function) noexcept :
        _priority(priority), _function(std::move(function))
    {
    }

    /**
     * @brief Creates a baseline object from the given cap height.
     *
     * This is for basic widgets where the object to be aligned is the only
     * object that will be rendered in the widget.
     *
     * @param priority The priority of the baseline.
     * @param cap_height The cap-height of the font in pixels.
     * @return The baseline object.
     */
    [[nodiscard]] static baseline from_cap_height(unsigned int priority, unit::pixels_f cap_height) noexcept
    {
        return hi::baseline{
            priority, [cap_height](unit::pixels_f height) {
                return baseline_function_result{unit::pixels(0.0f), height / 2.0f - cap_height / 2.0f, height - cap_height};
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
        unsigned int priority,
        unit::pixels_f cap_height,
        unit::pixels_f bottom_padding,
        unit::pixels_f top_padding) noexcept
    {
        return hi::baseline{priority, [cap_height, bottom_padding, top_padding](unit::pixels_f height) {
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
     * @param priority The priority of the new baseline function.
     * @param other The baseline function to embed.
     * @param bottom_padding The amount of bottom padding to add.
     * @param top_padding The amount of top padding to add.
     * @return The new embedded baseline function.
     */
    [[nodiscard]] static baseline
    embed(unsigned int priority, baseline const& other, unit::pixels_f bottom_padding, unit::pixels_f top_padding) noexcept
    {
        return hi::baseline{
            priority, [embedded_func = other._function, bottom_padding, top_padding](unit::pixels_f height) {
                auto const padded_height = height - bottom_padding - top_padding;
                auto const [bottom, middle, top] = embedded_func(padded_height);

                return baseline_function_result{bottom_padding + bottom, bottom_padding + middle, bottom_padding + top};
            }};
    }

    [[nodiscard]] constexpr unsigned int priority() const noexcept
    {
        return _priority;
    }

    /**
     * Calculates the baseline position based on the given height and alignment.
     *
     * @param height The height of the box in which an object must be aligned to
     *               the baseline.
     * @param alignment The vertical alignment of the object.
     * @return The baseline position in pixels from the bottom of the box.
     */
    [[nodiscard]] constexpr unit::pixels_f operator()(unit::pixels_f height, vertical_alignment alignment) const
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
     * Returns the baseline object with the highest priority.
     *
     * @param lhs The first baseline object.
     * @param rhs The second baseline object.
     * @return The baseline object with the higher priority.
     */
    [[nodiscard]] friend baseline max(baseline const& lhs, baseline const& rhs) noexcept
    {
        if (lhs._priority > rhs._priority) {
            return lhs;
        } else {
            return rhs;
        }
    }

private:
    unsigned int _priority = 0;
    baseline_function_type _function = {};
};
}
