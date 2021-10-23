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

void icon_widget::constrain() noexcept
{
    tt_axiom(is_gui_thread());

    _layout = {};

    ttlet icon_ = icon.cget();

    if (holds_alternative<std::monostate>(icon_)) {
        _icon_type = icon_type::no;
        _glyph = {};
        _pixmap_hash = 0;
        _pixmap_backing = {};
        // For uniform scaling issues, make sure the size is not zero.
        _icon_bounding_box = {};

    } else if (holds_alternative<pixel_map<sfloat_rgba16>>(icon_)) {
        // XXX very ugly, please fix.
        // This requires access to internals of vulkan, wtf.
        ttlet lock = std::scoped_lock(gfx_system_mutex);

        _icon_type = icon_type::pixmap;
        _glyph = {};

        ttlet &pixmap = get<pixel_map<sfloat_rgba16>>(icon_);

        gfx_device_vulkan *device = nullptr;
        if (window.surface) {
            device = narrow_cast<gfx_device_vulkan *>(window.surface->device());
        }

        if (device == nullptr) {
            // The window does not have a surface or device assigned.
            // We need a device to upload the image as texture map, so retry until it does.
            _pixmap_hash = 0;
            _pixmap_backing = {};
            _icon_bounding_box = {};
            request_reconstrain();

        } else if (pixmap.hash() != _pixmap_hash) {
            _pixmap_hash = pixmap.hash();
            _pixmap_backing = device->imagePipeline->makeImage(pixmap.width(), pixmap.height());

            _pixmap_backing.upload(pixmap);
            _icon_bounding_box = aarectangle{
                extent2{narrow_cast<float>(_pixmap_backing.width_in_px), narrow_cast<float>(_pixmap_backing.height_in_px)}};
        }

    } else if (holds_alternative<font_glyph_ids>(icon_)) {
        _icon_type = icon_type::glyph;
        _glyph = get<font_glyph_ids>(icon_);
        _pixmap_hash = 0;
        _pixmap_backing = {};

        _icon_bounding_box = _glyph.get_bounding_box() * theme().text_style(theme_text_style::label).scaled_size();

    } else if (holds_alternative<elusive_icon>(icon_)) {
        _icon_type = icon_type::glyph;
        _glyph = font_book().find_glyph(get<elusive_icon>(icon_));
        _pixmap_hash = 0;
        _pixmap_backing = {};

        _icon_bounding_box = _glyph.get_bounding_box() * theme().text_style(theme_text_style::label).scaled_size();

    } else if (holds_alternative<ttauri_icon>(icon_)) {
        _icon_type = icon_type::glyph;
        _glyph = font_book().find_glyph(get<ttauri_icon>(icon_));
        _pixmap_hash = 0;
        _pixmap_backing = {};

        _icon_bounding_box = _glyph.get_bounding_box() * theme().text_style(theme_text_style::label).scaled_size();

    } else {
        tt_no_default();
    }

    _minimum_size = {0.0f, 0.0f};
    _preferred_size = _icon_bounding_box.size();
    _maximum_size = _preferred_size;
    tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
}

void icon_widget::set_layout(layout_context const &context) noexcept
{
    tt_axiom(is_gui_thread());

    if (visible and _layout.store(context) >= layout_update::transform) {
        if (_icon_type == icon_type::no or not _icon_bounding_box) {
            _icon_transform = {};
        } else {
            _icon_transform = matrix2::uniform(_icon_bounding_box, layout().rectangle(), *alignment);
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
            switch (_pixmap_backing.state) {
            case pipeline_image::image::State::Drawing: request_redraw(); break;
            case pipeline_image::image::State::Uploaded: context.draw_image(layout(), _pixmap_backing, _icon_transform); break;
            default: break;
            }
            break;

        case icon_type::glyph: {
            ttlet box = _icon_transform * _icon_bounding_box;
            ttlet scale = box.width() / _icon_bounding_box.width();
            context.draw_glyph(layout(), _glyph, scale, box, theme().color(*color));
        } break;

        default: tt_no_default();
        }
    }
}

} // namespace tt
