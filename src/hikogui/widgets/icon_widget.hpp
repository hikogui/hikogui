// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/icon_widget.hpp Defines icon_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../GFX/GFX.hpp"
#include "../geometry/geometry.hpp"
#include "../l10n/l10n.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

hi_export_module(hikogui.widgets.icon_widget);

hi_export namespace hi {
inline namespace v1 {

template<typename Context>
concept icon_widget_attribute = forward_of<Context, observer<hi::icon>, observer<hi::alignment>, observer<hi::color>>;

/** An simple GUI widget that displays an icon.
 * @ingroup widgets
 *
 * The icon is scaled to the size of the widget,
 * parent widgets will use this scaling to set the correct size.
 */
class icon_widget : public widget {
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

    template<icon_widget_attribute... Attributes>
    icon_widget(widget_intf const* parent, Attributes&&... attributes) noexcept : icon_widget(parent)
    {
        set_attributes(std::forward<Attributes>(attributes)...);
    }

    void set_attributes() noexcept {}

    template<icon_widget_attribute First, icon_widget_attribute... Rest>
    void set_attributes(First&& first, Rest&&... rest) noexcept
    {
        if constexpr (forward_of<First, observer<hi::icon>>) {
            icon = std::forward<First>(first);
        } else if constexpr (forward_of<First, observer<hi::alignment>>) {
            alignment = std::forward<First>(first);
        } else if constexpr (forward_of<First, observer<hi::color>>) {
            color = std::forward<First>(first);
        } else {
            hi_static_no_default();
        }
        set_attributes(std::forward<Rest>(rest)...);
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

            if (auto const pixmap = std::get_if<hi::pixmap<sfloat_rgba16>>(&icon)) {
                _icon_type = icon_type::pixmap;
                _icon_size = extent2{narrow_cast<float>(pixmap->width()), narrow_cast<float>(pixmap->height())};

                if (not(_pixmap_backing = gfx_pipeline_image::paged_image{surface(), *pixmap})) {
                    // Could not get an image, retry.
                    _icon_has_modified = true;
                    ++global_counter<"icon_widget:no-backing-image:constrain">;
                    process_event({gui_event_type::window_reconstrain});
                }

            } else if (auto const g1 = std::get_if<font_book::font_glyph_type>(&icon)) {
                _glyph = *g1;
                _icon_type = icon_type::glyph;
                _icon_size = _glyph.get_metrics().bounding_rectangle.size() *
                    (theme().text_style(semantic_text_style::label)->size * theme().pixel_density).in(pixels_per_em);

            } else if (auto const g2 = std::get_if<elusive_icon>(&icon)) {
                _glyph = find_glyph(*g2);
                _icon_type = icon_type::glyph;
                _icon_size = _glyph.get_metrics().bounding_rectangle.size() *
                    (theme().text_style(semantic_text_style::label)->size * theme().pixel_density).in(pixels_per_em);

            } else if (auto const g3 = std::get_if<hikogui_icon>(&icon)) {
                _glyph = find_glyph(*g3);
                _icon_type = icon_type::glyph;
                _icon_size = _glyph.get_metrics().bounding_rectangle.size() *
                    (theme().text_style(semantic_text_style::label)->size * theme().pixel_density).in(pixels_per_em);
            }
        }

        auto const resolved_alignment = resolve(*alignment, os_settings::left_to_right());
        auto const icon_constraints = box_constraints{
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
                auto const width = std::clamp(context.shape.width(), minimum->width(), maximum->width());
                auto const height = std::clamp(context.shape.height(), minimum->height(), maximum->height());

                auto const icon_scale =
                    scale2::uniform(_icon_size, extent2{narrow_cast<float>(width), narrow_cast<float>(height)});
                auto const new_icon_size = narrow_cast<extent2>(icon_scale * _icon_size);
                auto const resolved_alignment = resolve(*alignment, os_settings::left_to_right());
                _icon_rectangle = align(context.rectangle(), new_icon_size, resolved_alignment);
            }
        }
    }
    void draw(draw_context const& context) noexcept override
    {
        if (mode() > widget_mode::invisible and overlaps(context, layout())) {
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

    icon_widget(widget_intf const* parent) noexcept : super(parent)
    {
        _icon_cbt = icon.subscribe([this](auto...) {
            _icon_has_modified = true;
            ++global_counter<"icon_widget:icon:constrain">;
            process_event({gui_event_type::window_reconstrain});
        });
    }
};

} // namespace v1
} // namespace hi::v1
