// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../color/color.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/matrix.hpp"
#include "../geometry/identity.hpp"
#include "../type_traits.hpp"

namespace tt {
class draw_context;
class icon;
class label;
struct text_style;

class stencil {
public:
    stencil(alignment alignment) :
        _alignment(alignment), _data_is_modified(true), _size_is_modified(true), _position_is_modified(true)
    {
    }

    virtual ~stencil() = default;
    stencil(stencil const &) noexcept = delete;
    stencil(stencil &&) noexcept = delete;
    stencil &operator=(stencil const &) noexcept = delete;
    stencil &operator=(stencil &&) noexcept = delete;

    /** Return the extent that this cell wants to be drawn as.
     */
    [[nodiscard]] virtual extent2 minimum_size() noexcept
    {
        return {};
    }

    /** Return the extent that this cell wants to be drawn as.
     */
    [[nodiscard]] virtual extent2 preferred_size() noexcept
    {
        return {};
    }

    /** Return the extent that this cell wants to be drawn as.
     */
    [[nodiscard]] virtual extent2 maximum_size() noexcept
    {
        return extent2::large();
    }

    /** Pass layout parameters in local coordinates.
     * @param rectangle The rectangle the stencil will be drawn into.
     * @param base_line_position The position of the base line within the rectangle.
     */
    virtual void set_layout_parameters(
        aarectangle const &rectangle,
        float base_line_position = std::numeric_limits<float>::infinity()) noexcept
    {
        if (_rectangle.size() != rectangle.size()) {
            _size_is_modified = true;
        }
        if (get<0>(_rectangle) != get<0>(rectangle) || _base_line_position != base_line_position) {
            _position_is_modified = true;
        }

        _rectangle = rectangle;
        _base_line_position =
            base_line_position != std::numeric_limits<float>::infinity() ? base_line_position : rectangle.middle();
    }

    /** Draw the cell.
     *
     * @param context The current draw context.
     * @param color The color to use for drawing.
     * @param transform The transformation to apply when drawing.
     * @return true if the stencil needs to be redrawn.
     */
    [[nodiscard]] virtual bool
    draw(draw_context context, tt::color color = tt::color{}, matrix3 transform = geo::identity()) noexcept = 0;

    [[nodiscard]] static std::unique_ptr<class image_stencil> make_unique(alignment alignment, icon const &icon);

    [[nodiscard]] static std::unique_ptr<class text_stencil>
    make_unique(alignment alignment, std::string const &text, text_style const &style);

    [[nodiscard]] static std::unique_ptr<class label_stencil>
    make_unique(alignment alignment, tt::label const &label, text_style const &style);

protected:
    alignment _alignment;
    aarectangle _rectangle;
    float _base_line_position;

    /** Set to true when the data of the cell has been modified.
     */
    bool _data_is_modified;
    bool _size_is_modified;
    bool _position_is_modified;
};

/** Draws a stencil and optionally request a redraw from the window.
 * @param stencil The stencil instance to call draw on.
 * @param context The current draw context.
 * @param color The color to use for drawing.
 * @param transform The transformation to apply when drawing.
 */
#define tt_stencil_draw(stencil, context, ...) \
    do { \
        if (tt_call_method(stencil, draw, context __VA_OPT__(, ) __VA_ARGS__)) {\
            this->window.request_redraw(aarectangle{context.transform() * context.clipping_rectangle()});\
        } \
    } while (false)

} // namespace tt
