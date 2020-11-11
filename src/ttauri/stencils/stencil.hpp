// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../aarect.hpp"

namespace tt {
class DrawContext;
class icon;
class label;
struct TextStyle;

class stencil {
public:
    stencil(alignment alignment) : _alignment(alignment), _data_is_modified(true), _layout_is_modified(true) {}
    virtual ~stencil() = default;
    stencil(stencil const &) noexcept = delete;
    stencil(stencil &&) noexcept = delete;
    stencil &operator=(stencil const &) noexcept = delete;
    stencil &operator=(stencil &&) noexcept = delete;

    /** Return the extent that this cell wants to be drawn as.
     */
    [[nodiscard]] virtual vec preferred_extent() noexcept
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
        _rectangle = rectangle;
        _base_line_position =
            base_line_position != std::numeric_limits<float>::infinity() ? base_line_position : rectangle.middle();
        _layout_is_modified = true;
    }

    /** Draw the cell.
     *
     * @param context The current draw context.
     * @param use_context_color True to use the colors in the context, False to use the colors in the cell itself.
     */
    virtual void draw(DrawContext context, bool use_context_color = false) noexcept = 0;

    [[nodiscard]] static std::unique_ptr<class image_stencil> make_unique(alignment alignment, icon const &icon);

    [[nodiscard]] static std::unique_ptr<class text_stencil>
    make_unique(alignment alignment, std::u8string const &text, TextStyle const &style);

    [[nodiscard]] static std::unique_ptr<class label_stencil>
    make_unique(alignment alignment, tt::label const &label, TextStyle const &style);

protected:
    alignment _alignment;
    aarect _rectangle;
    float _base_line_position;

    /** Set to true when the data of the cell has been modified.
     */
    bool _data_is_modified;
    bool _layout_is_modified;
};

} // namespace tt
