// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "icon_widget.hpp"
#include "../GFX/gfx_surface_vulkan.hpp"

namespace tt {

icon_widget::~icon_widget() {}

void icon_widget::init() noexcept
{
    _icon_callback = icon.subscribe([this]() {
        _request_constrain = true;
    });
}

[[nodiscard]] bool icon_widget::constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept
{
    tt_axiom(is_gui_thread());

    if (super::constrain(display_time_point, need_reconstrain)) {
        ttlet &icon_ = *icon;

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
                _request_constrain = true;

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

            _icon_bounding_box =
                scale(pipeline_SDF::device_shared::getBoundingBox(_glyph), theme::global(theme_text_style::label).scaled_size());

        } else {
            tt_no_default();
        }

        _minimum_size = {0.0f, 0.0f};
        _preferred_size = _icon_bounding_box.size();
        _maximum_size = _preferred_size;
        tt_axiom(_minimum_size <= _preferred_size && _preferred_size <= _maximum_size);
        return true;
    } else {
        return false;
    }
}

[[nodiscard]] void icon_widget::layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept
{
    tt_axiom(is_gui_thread());

    need_layout |= _request_layout.exchange(false);
    if (need_layout) {
        if (_icon_type == icon_type::no or not _icon_bounding_box) {
            _icon_transform = {};
        } else {
            _icon_transform = matrix2::uniform(_icon_bounding_box, rectangle(), *alignment);
        }
    }
    super::layout(displayTimePoint, need_layout);
}

void icon_widget::draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept
{
    tt_axiom(is_gui_thread());

    if (overlaps(context, _clipping_rectangle)) {
        switch (_icon_type) {
        case icon_type::no: break;

        case icon_type::pixmap:
            switch (_pixmap_backing.state) {
            case pipeline_image::Image::State::Drawing: request_redraw(); break;
            case pipeline_image::Image::State::Uploaded: context.draw_image(_pixmap_backing, _icon_transform); break;
            default: break;
            }
            break;

        case icon_type::glyph: context.draw_glyph(_glyph, _icon_transform * _icon_bounding_box, theme::global(*color)); break;

        default: tt_no_default();
        }
    }

    super::draw(std::move(context), display_time_point);
}

} // namespace tt
