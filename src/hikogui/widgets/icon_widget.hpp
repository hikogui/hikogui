// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/icon_widget.hpp Defines icon_widget.
 * @ingroup widgets
 */

#pragma once

#include "../GUI/module.hpp"
#include "../GFX/module.hpp"
#include "../geometry/module.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept icon_widget_attribute = forward_of<Context, observer<hi::icon>, observer<hi::alignment>>;

/** An simple GUI widget that displays an icon.
 * @ingroup widgets
 *
 * The icon is scaled to the size of the widget,
 * parent widgets will use this scaling to set the correct size.
 */
template<fixed_string Name = "">
class icon_widget final : public widget {
public:
    using super = widget;
    constexpr static auto prefix = Name / "icon";

    /** The icon to be displayed.
     */
    observer<icon> icon = hi::icon{};

    /** Alignment of the icon inside the widget.
     */
    observer<alignment> alignment = hi::alignment::middle_center();

    icon_widget(widget *parent, icon_widget_attribute auto&&...attributes) noexcept : icon_widget(parent)
    {
        set_attributes(hi_forward(attributes)...);
    }

    void set_attributes() noexcept {}
    void set_attributes(icon_widget_attribute auto&& first, icon_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::icon>>) {
            icon = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
        } else {
            hi_static_no_default();
        }
        set_attributes(hi_forward(rest)...);
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
    {
        if (_icon_has_modified.exchange(false)) {
            _icon_type = icon_type::no;
            _icon_size = {};
            _glyph = {};
            _pixmap_backing = {};

            if (hilet pixmap = std::get_if<hi::pixmap<sfloat_rgba16>>(&icon.read())) {
                _icon_type = icon_type::pixmap;
                _icon_size = extent2i{narrow_cast<int>(pixmap->width()), narrow_cast<int>(pixmap->height())};

                if (_pixmap_backing = paged_image{surface, *pixmap}; not _pixmap_backing) {
                    // Could not get an image, retry.
                    _icon_has_modified = true;
                    ++global_counter<"icon_widget:no-backing-image:constrain">;
                    process_event({gui_event_type::window_reconstrain});
                }

            } else if (hilet g1 = std::get_if<font_book::font_glyph_type>(&icon.read())) {
                _glyph = *g1;
                _icon_type = icon_type::glyph;
                _icon_size = narrow_cast<extent2i>(_glyph.get_bounding_rectangle().size() * theme<prefix>.line_height(this));

            } else if (hilet g2 = std::get_if<elusive_icon>(&icon.read())) {
                _glyph = find_glyph(*g2);
                _icon_type = icon_type::glyph;
                _icon_size = narrow_cast<extent2i>(_glyph.get_bounding_rectangle().size() * theme<prefix>.line_height(this));

            } else if (hilet g3 = std::get_if<hikogui_icon>(&icon.read())) {
                _glyph = find_glyph(*g3);
                _icon_type = icon_type::glyph;
                _icon_size = narrow_cast<extent2i>(_glyph.get_bounding_rectangle().size() * theme<prefix>.line_height(this));
            }
        }

        hilet resolved_alignment = resolve(*alignment, os_settings::left_to_right());
        hilet icon_constraints = box_constraints{
            extent2i{0, 0},
            _icon_size,
            _icon_size,
            resolved_alignment,
            theme<prefix>.margin(this)};
        return icon_constraints.constrain(*minimum, *maximum);
    }

    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(layout, context)) {
            if (_icon_type == icon_type::no or not _icon_size) {
                _icon_rectangle = {};
            } else {
                hilet widget_size = narrow_cast<extent2>(clamp(context.shape.size(), *minimum, *maximum));
                hilet original_icon_size = narrow_cast<extent2>(_icon_size);

                hilet icon_scale = scale2::uniform(original_icon_size, widget_size);
                hilet new_icon_size = narrow_cast<extent2i>(icon_scale * original_icon_size);

                hilet resolved_alignment = resolve(*alignment, os_settings::left_to_right());
                _icon_rectangle = align(context.rectangle(), new_icon_size, resolved_alignment);
            }
        }
    }

    void draw(widget_draw_context const& context) noexcept override
    {
        if (*mode > widget_mode::invisible and overlaps(context, layout)) {
            switch (_icon_type) {
            case icon_type::no:
                break;

            case icon_type::pixmap:
                if (not context.draw_image(layout, _icon_rectangle, _pixmap_backing)) {
                    // Continue redrawing until the image is loaded.
                    request_redraw();
                }
                break;

            case icon_type::glyph:
                {
                    context.draw_glyph(layout, _icon_rectangle, *_glyph.font, _glyph.glyph, theme<prefix>.fill_color(this));
                }
                break;

            default:
                hi_no_default();
            }
        }
    }
    /// @endprivatesection
private:
    enum class icon_type { no, glyph, pixmap };

    icon_type _icon_type;
    font_book::font_glyph_type _glyph;
    paged_image _pixmap_backing;
    decltype(icon)::callback_token _icon_cbt;
    std::atomic<bool> _icon_has_modified = true;

    extent2i _icon_size;
    aarectanglei _icon_rectangle;

    icon_widget(widget *parent) noexcept : super(parent)
    {
        _icon_cbt = icon.subscribe([this](auto...) {
            _icon_has_modified = true;
            ++global_counter<"icon_widget:icon:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });
    }
};

}} // namespace hi::v1
