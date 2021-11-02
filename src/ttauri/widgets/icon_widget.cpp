// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "icon_widget.hpp"
#include "../text/font_book.hpp"
#include "../GFX/gfx_surface_vulkan.hpp"
#include "../GFX/gfx_device_vulkan.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/theme.hpp"
#include "../cast.hpp"

namespace tt {

icon_widget::icon_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    icon.subscribe(_reconstrain_callback);
}

widget_constraints const &icon_widget::set_constraints() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};

    ttlet icon_ = icon.cget();

    if (holds_alternative<std::monostate>(icon_)) {
        _icon_type = icon_type::no;
        _icon_size = {};
        _glyph = {};
        _pixmap_hash = 0;
        _pixmap_backing = {};
        // For uniform scaling issues, make sure the size is not zero.

    } else if (ttlet pixmap = get_if<pixel_map<sfloat_rgba16>>(icon_)) {
        _icon_type = icon_type::pixmap;
        _icon_size = extent2{narrow_cast<float>(_pixmap_backing.width), narrow_cast<float>(_pixmap_backing.height)};
        _glyph = {};

        if (_pixmap_hash != pixmap.hash()) {
            // Pixmap has been changed, or has never been set.
            if (_pixmap_backing = window.make_image(pixmap.width(), pixmap.height())) {
                _pixmap_backing.upload(pixmap);
                _pixmap_hash = pixmap.hash();
            } else {
                window.request_reconstrain();
            }
        }

    } else if (ttlet g1 = get_if<font_glyph_ids>(icon_)) {
        _icon_type = icon_type::glyph;
        _glyph = *g1;
        _pixmap_hash = 0;
        _pixmap_backing = {};
        _icon_size = _glyph.get_bounding_box().size() * theme().text_style(theme_text_style::label).scaled_size();

    } else if (ttlet g2 = get_if<elusive_icon>(icon_)) {
        _icon_type = icon_type::glyph;
        _glyph = font_book().find_glyph(*g2);
        _pixmap_hash = 0;
        _pixmap_backing = {};
        _icon_size = _glyph.get_bounding_box().size() * theme().text_style(theme_text_style::label).scaled_size();

    } else if (ttlet g3 = get_if<ttauri_icon>(icon_)) {
        _icon_type = icon_type::glyph;
        _glyph = font_book().find_glyph(*g3);
        _pixmap_hash = 0;
        _pixmap_backing = {};
        _icon_size = _glyph.get_bounding_box().size() * theme().text_style(theme_text_style::label).scaled_size();

    } else {
        tt_no_default();
    }

    return _constraints = {extent2{0.0f, 0.0f}, _icon_size, _icon_size, theme().margin};
}

void icon_widget::set_layout(widget_layout const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and _layout.store(context) >= layout_update::transform) {
        if (_icon_type == icon_type::no or not _icon_size) {
            _icon_rectangle = {};
        } else {
            ttlet icon_scale = scale2::uniform(_icon_size, layout().size);
            ttlet new_icon_size = icon_scale * _icon_size;
            _icon_rectangle = align(layout().rectangle(), new_icon_size, *alignment);
        }
    }
}

void icon_widget::draw(draw_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and overlaps(context, layout())) {
        switch (_icon_type) {
        case icon_type::no: break;

        case icon_type::pixmap:
            if (not context.draw_image(layout(), _icon_rectangle, _pixmap_backing)) {
                request_redraw();
            }
            break;

        case icon_type::glyph: {
            context.draw_glyph(layout(), _icon_rectangle, theme().color(*color), _glyph);
        } break;

        default: tt_no_default();
        }
    }
}

} // namespace tt
