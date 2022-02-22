// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "icon_widget.hpp"
#include "../text/font_book.hpp"
#include "../GUI/gui_window.hpp"
#include "../GUI/theme.hpp"
#include "../cast.hpp"

namespace tt::inline v1 {

icon_widget::icon_widget(gui_window &window, widget *parent) noexcept : super(window, parent)
{
    _icon_callback_ptr = icon.subscribe([this]() {
        _icon_has_modified = true;
        this->request_reconstrain();
    });
    icon.subscribe(_reconstrain_callback);
}

widget_constraints const &icon_widget::set_constraints() noexcept
{
    _layout = {};

    if (_icon_has_modified.exchange(false)) {
        ttlet icon_ = icon.cget();
        _icon_type = icon_type::no;
        _icon_size = {};
        _glyph = {};
        _pixmap_backing = {};

        if (ttlet pixmap = get_if<pixel_map<sfloat_rgba16>>(&*icon_)) {
            _icon_type = icon_type::pixmap;
            _icon_size = extent2{narrow_cast<float>(pixmap->width()), narrow_cast<float>(pixmap->height())};

            if (not(_pixmap_backing = paged_image{window.surface.get(), *pixmap})) {
                // Could not get an image, retry.
                _icon_has_modified = true;
                request_reconstrain();
            }

        } else if (ttlet g1 = get_if<glyph_ids>(&*icon_)) {
            _glyph = *g1;
            _icon_type = icon_type::glyph;
            _icon_size = _glyph.get_bounding_box().size() * theme().text_style(theme_text_style::label).size * theme().scale;

        } else if (ttlet g2 = get_if<elusive_icon>(&*icon_)) {
            _glyph = font_book().find_glyph(*g2);
            _icon_type = icon_type::glyph;
            _icon_size = _glyph.get_bounding_box().size() * theme().text_style(theme_text_style::label).size * theme().scale;

        } else if (ttlet g3 = get_if<ttauri_icon>(&*icon_)) {
            _glyph = font_book().find_glyph(*g3);
            _icon_type = icon_type::glyph;
            _icon_size = _glyph.get_bounding_box().size() * theme().text_style(theme_text_style::label).size * theme().scale;
        }
    }
    return _constraints = {extent2{0.0f, 0.0f}, _icon_size, _icon_size, theme().margin};
}

void icon_widget::set_layout(widget_layout const &layout) noexcept
{
    if (compare_store(_layout, layout)) {
        if (_icon_type == icon_type::no or not _icon_size) {
            _icon_rectangle = {};
        } else {
            ttlet icon_scale = scale2::uniform(_icon_size, layout.size);
            ttlet new_icon_size = icon_scale * _icon_size;
            _icon_rectangle = align(layout.rectangle(), new_icon_size, *alignment);
        }
    }
}

void icon_widget::draw(draw_context const &context) noexcept
{
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

} // namespace tt::inline v1
