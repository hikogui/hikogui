// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/widgets/widget.hpp"
#include "ttauri/widgets/grid_widget.hpp"
#include "ttauri/widgets/row_column_widget.hpp"
#include "ttauri/widgets/toggle_widget.hpp"
#include "ttauri/widgets/momentary_button_widget.hpp"
#include "ttauri/widgets/selection_widget.hpp"
#include "ttauri/widgets/radio_button_widget.hpp"
#include "ttauri/text/font_book.hpp"
#include "ttauri/codec/png.hpp"
#include "ttauri/GFX/RenderDoc.hpp"
#include "ttauri/crt.hpp"
#include <numbers>

enum class drawing_type {
    box,
    lines,
    circle,
    glyph,
    image,
};

auto drawing_list = std::vector<std::pair<drawing_type, tt::label>>{
    {drawing_type::box, tt::l10n("Box")},
    {drawing_type::lines, tt::l10n("Lines")},
    {drawing_type::circle, tt::l10n("Circle")},
    {drawing_type::glyph, tt::l10n("Glyph")},
    {drawing_type::image, tt::l10n("Image")},
};

enum class shape_type { square, rectangle, convex, concave, glyph_aspect_ratio, image_aspect_ratio };

auto shape_list = std::vector<std::pair<shape_type, tt::label>>{
    {shape_type::square, tt::l10n("Square")},
    {shape_type::rectangle, tt::l10n("Rectangle")},
    {shape_type::convex, tt::l10n("Convex")},
    {shape_type::concave, tt::l10n("Concave")},
    {shape_type::glyph_aspect_ratio, tt::l10n("Glyph Aspect Ratio")},
    {shape_type::image_aspect_ratio, tt::l10n("Image Aspect Ratio")},
};

enum class gradient_type { solid, horizontal, vertical, corners };

auto gradient_list = std::vector<std::pair<gradient_type, tt::label>>{
    {gradient_type::solid, tt::l10n("Solid")},
    {gradient_type::horizontal, tt::l10n("Horizontal")},
    {gradient_type::vertical, tt::l10n("Vertical")},
    {gradient_type::corners, tt::l10n("Corners")},
};

auto border_width_list = std::vector<std::pair<float, tt::label>>{
    {0.0f, tt::l10n("no border")},
    {1.0f, tt::l10n("1 px")},
    {2.0f, tt::l10n("2 px")},
    {4.0f, tt::l10n("4 px")},
    {8.0f, tt::l10n("8 px")},
};

// Every widget must inherit from tt::widget.
class drawing_widget : public tt::widget {
public:
    constexpr static auto blue = tt::color(0.05f, 0.05f, 0.50f);
    constexpr static auto red = tt::color(0.50f, 0.05f, 0.05f);
    constexpr static auto cyan = tt::color(0.05f, 0.50f, 0.50f);
    constexpr static auto white = tt::color(0.50f, 0.50f, 0.50f);
    constexpr static auto redish = tt::color(0.70f, 0.30f, 0.00f);
    constexpr static auto greenish = tt::color(0.00f, 0.30f, 0.70f);
    constexpr static auto blueish = tt::color(0.00f, 0.70f, 0.30f);
    constexpr static auto redish2 = tt::color(0.70f, 0.00f, 0.30f);

    tt::observable<drawing_type> drawing = drawing_type::box;
    tt::observable<shape_type> shape = shape_type::square;
    tt::observable<gradient_type> gradient = gradient_type::solid;
    tt::observable<bool> rotating = false;
    tt::observable<bool> clip = false;
    tt::observable<tt::border_side> border_side = tt::border_side::on;
    tt::observable<float> border_width = 0.0f;
    tt::observable<bool> rounded = false;

    // Every constructor of a widget starts with a `window` and `parent` argument.
    // In most cases these are automatically filled in when calling a container widget's `make_widget()` function.
    drawing_widget(tt::gui_window &window, tt::widget *parent) noexcept :
        widget(window, parent), _image(tt::URL("resource:mars3.png"))
    {
        this->drawing.subscribe(_redraw_callback);
        this->shape.subscribe(_redraw_callback);
        this->gradient.subscribe(_redraw_callback);
        this->rotating.subscribe(_redraw_callback);
        this->clip.subscribe(_redraw_callback);
        this->border_side.subscribe(_redraw_callback);
        this->border_width.subscribe(_redraw_callback);
        this->rounded.subscribe(_redraw_callback);

        this->_glyph = font_book().find_glyph(tt::elusive_icon::Briefcase);
    }

    // The set_constraints() function is called when the window is first initialized,
    // or when a widget wants to change its constraints.
    tt::widget_constraints const &set_constraints() noexcept override
    {
        // Almost all widgets will reset the `_layout` variable here so that it will
        // trigger the calculations in `set_layout()` as well.
        _layout = {};

        if (_image_was_modified.exchange(false)) {
            if (not(_image_backing = tt::paged_image{window.surface.get(), _image})) {
                // Could not get an image, retry.
                _image_was_modified = true;
                request_reconstrain();
            }
        }

        // Certain expensive calculations, such as loading of images and shaping of text
        // can be done in this function.

        // The constrains below have different minimum, preferred and maximum sizes.
        // When the window is initially created it will try to size itself so that
        // the contained widgets are at their preferred size. Having a different minimum
        // and/or maximum size will allow the window to be resizable.
        return _constraints = {{100, 100}, {150, 150}, {400, 400}, theme().margin};
    }

    // The `set_layout()` function is called when the window has resized, or when
    // a widget wants to change the internal layout.
    //
    // NOTE: The size of the layout may be larger than the maximum constraints of this widget.
    void set_layout(tt::widget_layout const &layout) noexcept override
    {
        // Update the `_layout` with the new context, in this case we want to do some
        // calculations when the size of the widget was changed.
        if (compare_store(_layout, layout)) {
            // Make a size scaled to the layout.
            auto const max_size = _layout.size * 0.9f;
            auto const max_rectangle = tt::aarectangle{tt::point2{max_size.width() * -0.5f, max_size.height() * -0.5f}, max_size};

            // Here we can do some semi-expensive calculations which must be done when resizing the widget.
            // In this case we make two rectangles which are used in the `draw()` function.
            auto const glyph_size = _glyph.get_bounding_box().size();
            auto const glyph_scale = tt::scale2::uniform(glyph_size, max_size);
            auto const new_glyph_size = glyph_scale * glyph_size;
            _glyph_rectangle = align(max_rectangle, new_glyph_size, tt::alignment::middle_center());

            auto const image_size = tt::extent2{static_cast<float>(_image.width()), static_cast<float>(_image.height())};
            auto const image_scale = tt::scale2::uniform(image_size, max_size);
            auto const new_image_size = image_scale * image_size;
            _image_rectangle = align(max_rectangle, new_image_size, tt::alignment::middle_center());
        }
    }

    [[nodiscard]] tt::quad_color fill_color() const noexcept
    {
        switch (*gradient) {
        case gradient_type::solid: return tt::quad_color(blue);
        case gradient_type::horizontal: return tt::quad_color{blue, red, blue, red};
        case gradient_type::vertical: return tt::quad_color{blue, blue, red, red};
        case gradient_type::corners: return tt::quad_color{red, blue, cyan, white};
        default: tt_no_default();
        }
    }

    [[nodiscard]] tt::quad_color line_color() const noexcept
    {
        if (border_width == 0.0f) {
            // Due to inaccuracies in the shaders, a thin border may present itself inside
            // the anti-aliased edges; so make it the same color as the fill. This is the
            // same thing that happens when you call draw_box with only a fill color.
            return fill_color();
        } else {
            switch (*gradient) {
            case gradient_type::solid: return tt::quad_color(redish);
            case gradient_type::horizontal: return tt::quad_color{redish, greenish, redish, greenish};
            case gradient_type::vertical: return tt::quad_color{redish, redish, greenish, greenish};
            case gradient_type::corners: return tt::quad_color{redish, greenish, blueish, redish2};
            default: tt_no_default();
            }
        }
    }

    [[nodiscard]] tt::quad shape_quad() const noexcept
    {
        switch (*shape) {
        case shape_type::square:
            return tt::quad{
                tt::point3{-40.0f, -40.0f}, tt::point3{40.0f, -40.0f}, tt::point3{-40.0f, 40.0f}, tt::point3{40.0f, 40.0f}};
        case shape_type::rectangle:
            return tt::quad{
                tt::point3{-50.0f, -40.0f}, tt::point3{50.0f, -40.0f}, tt::point3{-50.0f, 40.0f}, tt::point3{50.0f, 40.0f}};
        case shape_type::convex:
            return tt::quad{
                tt::point3{-50.0f, -10.0f}, tt::point3{50.0f, -40.0f}, tt::point3{-50.0f, 40.0f}, tt::point3{50.0f, 50.0f}};
        case shape_type::concave:
            return tt::quad{
                tt::point3{20.0f, 20.0f}, tt::point3{50.0f, -40.0f}, tt::point3{-50.0f, 40.0f}, tt::point3{50.0f, 50.0f}};
        case shape_type::glyph_aspect_ratio: return _glyph_rectangle;
        case shape_type::image_aspect_ratio: return _image_rectangle;
        default: tt_no_default();
        }
    }

    [[nodiscard]] tt::rotate3 rotation(tt::draw_context const &context) const noexcept
    {
        float angle = 0.0f;
        if (rotating) {
            request_redraw();
            constexpr auto interval_in_ns = 10'000'000'000;
            auto const repeating_interval = context.display_time_point.time_since_epoch().count() % interval_in_ns;
            angle = (float(repeating_interval) / float(interval_in_ns)) * 2.0f * std::numbers::pi_v<float>;
        }
        return tt::rotate3(angle, tt::vector3{0.0f, 0.0f, 1.0f});
    }

    [[nodiscard]] tt::corner_radii corners() const noexcept
    {
        if (*rounded) {
            return tt::corner_radii{20.0f, 10.0f, 5.0f, 0.0f};
        } else {
            return {};
        }
    }

    [[nodiscard]] tt::line_end_cap end_cap() const noexcept
    {
        return *rounded ? tt::line_end_cap::round : tt::line_end_cap::flat;
    }

    // The `draw()` function is called when all or part of the window requires redrawing.
    // This may happen when showing the window for the first time, when the operating-system
    // requests a (partial) redraw, or when a widget requests a redraw of itself.
    void draw(tt::draw_context const &context) noexcept override
    {
        using namespace std::chrono_literals;

        auto const clipping_rectangle =
            clip ? tt::aarectangle{0.0f, 0.0f, _layout.width(), _layout.height() * 0.5f} : _layout.rectangle();

        auto const translation = tt::translate3(std::floor(_layout.width() * 0.5f), std::floor(_layout.height()) * 0.5f, 0.0f);
        auto const transform = translation * rotation(context);

        auto const circle = tt::circle{tt::point3{0.0f, 0.0f, 0.0f}, 50.0f};

        // We only need to draw the widget when it is visible and when the visible area of
        // the widget overlaps with the scissor-rectangle (partial redraw) of the drawing context.
        if (visible and overlaps(context, layout())) {
            switch (*drawing) {
            case drawing_type::box:
                context.draw_box(
                    _layout,
                    clipping_rectangle,
                    transform * shape_quad(),
                    fill_color(),
                    line_color(),
                    *border_width,
                    *border_side,
                    corners());
                break;

            case drawing_type::lines: {
                // There is a concave corner at left-bottom, so I want this to be the second point the lines pass through.
                auto const quad = shape_quad();
                auto const line1 = tt::line_segment{get<0>(quad), get<1>(quad)};
                auto const line2 = tt::line_segment{get<0>(quad), get<2>(quad)};
                auto const line3 = tt::line_segment{get<3>(quad), get<2>(quad)};
                auto const width = std::max(*border_width, 0.5f);
                context.draw_line(_layout, clipping_rectangle, transform * line1, width, fill_color(), end_cap(), end_cap());
                context.draw_line(_layout, clipping_rectangle, transform * line2, width, fill_color(), end_cap(), end_cap());
                context.draw_line(_layout, clipping_rectangle, transform * line3, width, fill_color(), end_cap(), end_cap());
            } break;

            case drawing_type::circle:
                context.draw_circle(
                    _layout, clipping_rectangle, translation * circle, fill_color(), line_color(), *border_width, *border_side);
                break;

            case drawing_type::glyph:
                // A full rectangle is visible.
                context.draw_glyph(_layout, clipping_rectangle, transform * shape_quad(), fill_color(), _glyph);
                break;

            case drawing_type::image:
                if (not context.draw_image(_layout, clipping_rectangle, transform * shape_quad(), _image_backing)) {
                    // Image was not yet uploaded to the texture atlas, redraw until it does.
                    request_redraw();
                }
                break;

            default: tt_no_default();
            }
        }
    }

private:
    tt::glyph_ids _glyph;
    tt::aarectangle _glyph_rectangle;
    std::atomic<bool> _image_was_modified = true;
    tt::png _image;
    tt::aarectangle _image_rectangle;
    tt::paged_image _image_backing;
};

int tt_main(int argc, char *argv[])
{
    tt::observable<drawing_type> drawing = drawing_type::box;
    tt::observable<shape_type> shape = shape_type::square;
    tt::observable<bool> rotating = false;
    tt::observable<bool> clip = false;
    tt::observable<gradient_type> gradient = gradient_type::solid;
    tt::observable<tt::border_side> border_side = tt::border_side::on;
    tt::observable<float> border_width = 0.0f;
    tt::observable<bool> rounded = false;

    // Startup renderdoc for debugging
    auto render_doc = tt::RenderDoc();

    auto gui = tt::gui_system::make_unique();
    auto &window = gui->make_window(tt::l10n("Drawing Custom Widget"));

    auto &custom_widget = window.content().make_widget<drawing_widget>("A1:D1");
    custom_widget.drawing = drawing;
    custom_widget.shape = shape;
    custom_widget.rotating = rotating;
    custom_widget.clip = clip;
    custom_widget.gradient = gradient;
    custom_widget.border_side = border_side;
    custom_widget.border_width = border_width;
    custom_widget.rounded = rounded;

    window.content().make_widget<tt::label_widget>("A2", tt::l10n("Drawing type:"));
    window.content().make_widget<tt::selection_widget>("B2:D2", drawing_list, drawing);

    window.content().make_widget<tt::label_widget>("A3", tt::l10n("Shape:"));
    window.content().make_widget<tt::selection_widget>("B3:D3", shape_list, shape);

    window.content().make_widget<tt::label_widget>("A4", tt::l10n("Gradient:"));
    window.content().make_widget<tt::selection_widget>("B4:D4", gradient_list, gradient);

    window.content().make_widget<tt::label_widget>("A5", tt::l10n("Border side:"));
    window.content().make_widget<tt::radio_button_widget>("B5", tt::l10n("on"), border_side, tt::border_side::on);
    window.content().make_widget<tt::radio_button_widget>("C5", tt::l10n("inside"), border_side, tt::border_side::inside);
    window.content().make_widget<tt::radio_button_widget>("D5", tt::l10n("outside"), border_side, tt::border_side::outside);

    window.content().make_widget<tt::label_widget>("A6", tt::l10n("Border width:"));
    window.content().make_widget<tt::selection_widget>("B6:D6", border_width_list, border_width);

    window.content().make_widget<tt::label_widget>("A7", tt::l10n("Rotate:"));
    window.content().make_widget<tt::toggle_widget>("B7:D7", rotating);

    window.content().make_widget<tt::label_widget>("A8", tt::l10n("Clip:"));
    window.content().make_widget<tt::toggle_widget>("B8:D8", clip);

    window.content().make_widget<tt::label_widget>("A9", tt::l10n("Rounded:"));
    window.content().make_widget<tt::toggle_widget>("B9:D9", rounded);

    return gui->loop();
}
