// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "icon_widget.hpp"
#include "../font/module.hpp"
#include "../GUI/theme.hpp"
#include "../utility/module.hpp"

namespace hi::inline v1 {

icon_widget::icon_widget(widget *parent) noexcept : super(parent)
{
    _icon_cbt = icon.subscribe([this](auto...) {
        _icon_has_modified = true;
        ++global_counter<"icon_widget:icon:constrain">;
        process_event({gui_event_type::window_reconstrain});
    });
}

[[nodiscard]] box_constraints icon_widget::update_constraints() noexcept
{
    _layout = {};

    if (_icon_has_modified.exchange(false)) {
        _icon_type = icon_type::no;
        _icon_size = {};
        _glyph = {};
        _pixmap_backing = {};

        if (hilet pixmap = std::get_if<hi::pixmap<sfloat_rgba16>>(&icon.read())) {
            _icon_type = icon_type::pixmap;
            _icon_size = extent2{narrow_cast<float>(pixmap->width()), narrow_cast<float>(pixmap->height())};

            if (not(_pixmap_backing = paged_image{surface(), *pixmap})) {
                // Could not get an image, retry.
                _icon_has_modified = true;
                ++global_counter<"icon_widget:no-backing-image:constrain">;
                process_event({gui_event_type::window_reconstrain});
            }

        } else if (hilet g1 = std::get_if<glyph_ids>(&icon.read())) {
            _glyph = *g1;
            _icon_type = icon_type::glyph;
            _icon_size = _glyph.get_bounding_box().size() * theme().text_style(semantic_text_style::label)->size * theme().scale;

        } else if (hilet g2 = std::get_if<elusive_icon>(&icon.read())) {
            _glyph = find_glyph(*g2);
            _icon_type = icon_type::glyph;
            _icon_size = _glyph.get_bounding_box().size() * theme().text_style(semantic_text_style::label)->size * theme().scale;

        } else if (hilet g3 = std::get_if<hikogui_icon>(&icon.read())) {
            _glyph = find_glyph(*g3);
            _icon_type = icon_type::glyph;
            _icon_size = _glyph.get_bounding_box().size() * theme().text_style(semantic_text_style::label)->size * theme().scale;
        }
    }

    hilet resolved_alignment = resolve(*alignment, os_settings::left_to_right());
    hilet icon_constraints = box_constraints{
        extent2i{0, 0},
        narrow_cast<extent2i>(_icon_size),
        narrow_cast<extent2i>(_icon_size),
        resolved_alignment,
        theme().margin<int>()};
    return icon_constraints.constrain(*minimum, *maximum);
}

void icon_widget::set_layout(widget_layout const& context) noexcept
{
    if (compare_store(_layout, context)) {
        if (_icon_type == icon_type::no or not _icon_size) {
            _icon_rectangle = {};
        } else {
            hilet width = std::clamp(context.shape.width(), minimum->width(), maximum->width());
            hilet height = std::clamp(context.shape.height(), minimum->height(), maximum->height());

            hilet icon_scale = scale2::uniform(_icon_size, extent2{narrow_cast<float>(width), narrow_cast<float>(height)});
            hilet new_icon_size = narrow_cast<extent2i>(icon_scale * _icon_size);
            hilet resolved_alignment = resolve(*alignment, os_settings::left_to_right());
            _icon_rectangle = align(context.rectangle(), new_icon_size, resolved_alignment);
        }
    }
}

void icon_widget::draw(draw_context const& context) noexcept
{
    if (*mode > widget_mode::invisible and overlaps(context, layout())) {
        switch (_icon_type) {
        case icon_type::no:
            break;

        case icon_type::pixmap:
            if (not context.draw_image(layout(), _icon_rectangle, _pixmap_backing)) {
                // Continue redrawing until the image is loaded.
                request_redraw();
            }
            break;

        case icon_type::glyph:
            {
                context.draw_glyph(layout(), _icon_rectangle, _glyph, theme().color(*color));
            }
            break;

        default:
            hi_no_default();
        }
    }
}

} // namespace hi::inline v1
