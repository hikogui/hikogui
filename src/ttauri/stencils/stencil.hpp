// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../aarect.hpp"

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
    [[nodiscard]] virtual extent2 preferred_extent() noexcept
    {
        return {};
    }

    /** Pass layout parameters in local coordinates.
     * @param rectangle The rectangle the stencil will be drawn into.
     * @param base_line_position The position of the base line within the rectangle.
     */
    virtual void
    set_layout_parameters(aarect const &rectangle, float base_line_position = std::numeric_limits<float>::infinity()) noexcept
    {
        if (_rectangle.extent() != rectangle.extent()) {
            _size_is_modified = true;
        }
        if (_rectangle.offset() != rectangle.offset() || _base_line_position != base_line_position) {
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
     */
    virtual void draw(draw_context context, tt::color color = tt::color{}, matrix3 transform = geo::identity()) noexcept = 0;

    [[nodiscard]] static std::unique_ptr<class image_stencil> make_unique(alignment alignment, icon const &icon);

    [[nodiscard]] static std::unique_ptr<class text_stencil>
    make_unique(alignment alignment, std::u8string const &text, text_style const &style);

    [[nodiscard]] static std::unique_ptr<class label_stencil>
    make_unique(alignment alignment, tt::label const &label, text_style const &style);

protected:
    alignment _alignment;
    aarect _rectangle;
    float _base_line_position;

    /** Set to true when the data of the cell has been modified.
     */
    bool _data_is_modified;
    bool _size_is_modified;
    bool _position_is_modified;
};

} // namespace tt
