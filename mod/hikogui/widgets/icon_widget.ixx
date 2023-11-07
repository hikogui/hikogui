// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/icon_widget.hpp Defines icon_widget.
 * @ingroup widgets
 */

module;
#include "../macros.hpp"

#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

export module hikogui_widgets_icon_widget;
import hikogui_GFX;
import hikogui_geometry;
import hikogui_l10n;
import hikogui_widgets_widget;

export namespace hi { inline namespace v1 {

template<typename Context>
concept icon_widget_attribute = forward_of<Context, observer<hi::icon>, observer<hi::alignment>, observer<hi::color>>;

/** An simple GUI widget that displays an icon.
 * @ingroup widgets
 *
 * The icon is scaled to the size of the widget,
 * parent widgets will use this scaling to set the correct size.
 */
class icon_widget final : public widget {
public:
    using super = widget;

    /** The icon to be displayed.
     */
    observer<icon> icon = hi::icon{};

    /** The color a non-color icon will be displayed with.
     */
    observer<color> color = color::foreground();

    /** Alignment of the icon inside the widget.
     */
    observer<alignment> alignment = hi::alignment::middle_center();

    icon_widget(not_null<widget_intf const *> parent, icon_widget_attribute auto&&...attributes) noexcept :
        icon_widget(parent)
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
        } else if constexpr (forward_of<decltype(first), observer<hi::color>>) {
            color = hi_forward(first);
        } else {
            hi_static_no_default();
        }
        set_attributes(hi_forward(rest)...);
    }

    /// @privatesection
    [[nodiscard]] box_constraints update_constraints() noexcept override
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

                if (not(_pixmap_backing = gfx_pipeline_image::paged_image{surface(), *pixmap})) {
                    // Could not get an image, retry.
                    _icon_has_modified = true;
                    ++global_counter<"icon_widget:no-backing-image:constrain">;
                    process_event({gui_event_type::window_reconstrain});
                }

            } else if (hilet g1 = std::get_if<font_book::font_glyph_type>(&icon.read())) {
                _glyph = *g1;
                _icon_type = icon_type::glyph;
                _icon_size =
                    _glyph.get_metrics().bounding_rectangle.size() * theme().text_style(semantic_text_style::label)->size * theme().scale;

            } else if (hilet g2 = std::get_if<elusive_icon>(&icon.read())) {
                _glyph = find_glyph(*g2);
                _icon_type = icon_type::glyph;
                _icon_size =
                    _glyph.get_metrics().bounding_rectangle.size() * theme().text_style(semantic_text_style::label)->size * theme().scale;

            } else if (hilet g3 = std::get_if<hikogui_icon>(&icon.read())) {
                _glyph = find_glyph(*g3);
                _icon_type = icon_type::glyph;
                _icon_size =
                    _glyph.get_metrics().bounding_rectangle.size() * theme().text_style(semantic_text_style::label)->size * theme().scale;
            }
        }

        hilet resolved_alignment = resolve(*alignment, os_settings::left_to_right());
        hilet icon_constraints = box_constraints{
            extent2{0, 0},
            narrow_cast<extent2>(_icon_size),
            narrow_cast<extent2>(_icon_size),
            resolved_alignment,
            theme().margin<float>()};
        return icon_constraints.constrain(*minimum, *maximum);
    }
    void set_layout(widget_layout const& context) noexcept override
    {
        if (compare_store(_layout, context)) {
            if (_icon_type == icon_type::no or not _icon_size) {
                _icon_rectangle = {};
            } else {
                hilet width = std::clamp(context.shape.width(), minimum->width(), maximum->width());
                hilet height = std::clamp(context.shape.height(), minimum->height(), maximum->height());

                hilet icon_scale = scale2::uniform(_icon_size, extent2{narrow_cast<float>(width), narrow_cast<float>(height)});
                hilet new_icon_size = narrow_cast<extent2>(icon_scale * _icon_size);
                hilet resolved_alignment = resolve(*alignment, os_settings::left_to_right());
                _icon_rectangle = align(context.rectangle(), new_icon_size, resolved_alignment);
            }
        }
    }
    void draw(draw_context const& context) noexcept override
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
    /// @endprivatesection
private:
    enum class icon_type { no, glyph, pixmap };

    icon_type _icon_type;
    font_book::font_glyph_type _glyph;
    gfx_pipeline_image::paged_image _pixmap_backing;
    std::atomic<bool> _icon_has_modified = true;

    extent2 _icon_size;
    aarectangle _icon_rectangle;

    callback<void(hi::icon)> _icon_cbt;

    icon_widget(not_null<widget_intf const *> parent) noexcept : super(parent)
    {
        _icon_cbt = icon.subscribe([this](auto...) {
            _icon_has_modified = true;
            ++global_counter<"icon_widget:icon:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });
    }
};

}} // namespace hi::v1
