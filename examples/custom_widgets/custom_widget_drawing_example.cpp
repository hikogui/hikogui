// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/GFX/RenderDoc.hpp"
#include "hikogui/GUI/gui_system.hpp"
#include "hikogui/widgets/widget.hpp"
#include "hikogui/widgets/grid_widget.hpp"
#include "hikogui/widgets/row_column_widget.hpp"
#include "hikogui/widgets/toggle_widget.hpp"
#include "hikogui/widgets/momentary_button_widget.hpp"
#include "hikogui/widgets/selection_widget.hpp"
#include "hikogui/widgets/radio_button_widget.hpp"
#include "hikogui/text/font_book.hpp"
#include "hikogui/codec/png.hpp"
#include "hikogui/file/URL.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/loop.hpp"
#include <numbers>

enum class drawing_type {
    box,
    lines,
    circle,
    glyph,
    image,
};

auto drawing_list = std::vector<std::pair<drawing_type, hi::label>>{
    {drawing_type::box, hi::tr("Box")},
    {drawing_type::lines, hi::tr("Lines")},
    {drawing_type::circle, hi::tr("Circle")},
    {drawing_type::glyph, hi::tr("Glyph")},
    {drawing_type::image, hi::tr("Image")},
};

enum class shape_type { square, rectangle, convex, concave, glyph_aspect_ratio, image_aspect_ratio };

auto shape_list = std::vector<std::pair<shape_type, hi::label>>{
    {shape_type::square, hi::tr("Square")},
    {shape_type::rectangle, hi::tr("Rectangle")},
    {shape_type::convex, hi::tr("Convex")},
    {shape_type::concave, hi::tr("Concave")},
    {shape_type::glyph_aspect_ratio, hi::tr("Glyph Aspect Ratio")},
    {shape_type::image_aspect_ratio, hi::tr("Image Aspect Ratio")},
};

enum class gradient_type { solid, horizontal, vertical, corners };

auto gradient_list = std::vector<std::pair<gradient_type, hi::label>>{
    {gradient_type::solid, hi::tr("Solid")},
    {gradient_type::horizontal, hi::tr("Horizontal")},
    {gradient_type::vertical, hi::tr("Vertical")},
    {gradient_type::corners, hi::tr("Corners")},
};

auto border_width_list = std::vector<std::pair<float, hi::label>>{
    {0.0f, hi::tr("no border")},
    {1.0f, hi::tr("1 px")},
    {2.0f, hi::tr("2 px")},
    {4.0f, hi::tr("4 px")},
    {8.0f, hi::tr("8 px")},
};

// Every widget must inherit from hi::widget.
class drawing_widget : public hi::widget {
public:
    constexpr static auto blue = hi::color(0.05f, 0.05f, 0.50f);
    constexpr static auto red = hi::color(0.50f, 0.05f, 0.05f);
    constexpr static auto cyan = hi::color(0.05f, 0.50f, 0.50f);
    constexpr static auto white = hi::color(0.50f, 0.50f, 0.50f);
    constexpr static auto redish = hi::color(0.70f, 0.30f, 0.00f);
    constexpr static auto greenish = hi::color(0.00f, 0.30f, 0.70f);
    constexpr static auto blueish = hi::color(0.00f, 0.70f, 0.30f);
    constexpr static auto redish2 = hi::color(0.70f, 0.00f, 0.30f);

    hi::observer<drawing_type> drawing = drawing_type::box;
    hi::observer<shape_type> shape = shape_type::square;
    hi::observer<gradient_type> gradient = gradient_type::solid;
    hi::observer<bool> rotating = false;
    hi::observer<bool> clip = false;
    hi::observer<hi::border_side> border_side = hi::border_side::on;
    hi::observer<float> border_width = 0.0f;
    hi::observer<bool> rounded = false;

    // Every constructor of a widget starts with a `window` and `parent` argument.
    // In most cases these are automatically filled in when calling a container widget's `make_widget()` function.
    drawing_widget(hi::gui_window& window, hi::widget *parent) noexcept :
        widget(window, parent), _image(hi::URL("resource:mars3.png"))
    {
        // clang-format off
        _drawing_cbt = this->drawing.subscribe([&](auto...){ request_redraw(); });
        _shape_cbt = this->shape.subscribe([&](auto...){ request_redraw(); });
        _gradient_cbt = this->gradient.subscribe([&](auto...){ request_redraw(); });
        _rotating_cbt = this->rotating.subscribe([&](auto...){ request_redraw(); });
        _clip_cbt = this->clip.subscribe([&](auto...){ request_redraw(); });
        _border_side_cbt = this->border_side.subscribe([&](auto...){ request_redraw(); });
        _border_width_cbt = this->border_width.subscribe([&](auto...){ request_redraw(); });
        _rounded_cbt = this->rounded.subscribe([&](auto...){ request_redraw(); });
        // clang-format on

        this->_glyph = font_book().find_glyph(hi::elusive_icon::Briefcase);
    }

    // The set_constraints() function is called when the window is first initialized,
    // or when a widget wants to change its constraints.
    hi::widget_constraints const& set_constraints() noexcept override
    {
        // Almost all widgets will reset the `_layout` variable here so that it will
        // trigger the calculations in `set_layout()` as well.
        _layout = {};

        if (_image_was_modified.exchange(false)) {
            if (not(_image_backing = hi::paged_image{window.surface.get(), _image})) {
                // Could not get an image, retry.
                _image_was_modified = true;
                hi_request_reconstrain("drawing_widget::set_constraints() could not get backing image.");
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
    void set_layout(hi::widget_layout const& layout) noexcept override
    {
        // Update the `_layout` with the new context, in this case we want to do some
        // calculations when the size of the widget was changed.
        if (compare_store(_layout, layout)) {
            // Make a size scaled to the layout.
            auto const max_size = _layout.size * 0.9f;
            auto const max_rectangle = hi::aarectangle{hi::point2{max_size.width() * -0.5f, max_size.height() * -0.5f}, max_size};

            // Here we can do some semi-expensive calculations which must be done when resizing the widget.
            // In this case we make two rectangles which are used in the `draw()` function.
            auto const glyph_size = _glyph.get_bounding_box().size();
            auto const glyph_scale = hi::scale2::uniform(glyph_size, max_size);
            auto const new_glyph_size = glyph_scale * glyph_size;
            _glyph_rectangle = align(max_rectangle, new_glyph_size, hi::alignment::middle_center());

            auto const image_size = hi::extent2{static_cast<float>(_image.width()), static_cast<float>(_image.height())};
            auto const image_scale = hi::scale2::uniform(image_size, max_size);
            auto const new_image_size = image_scale * image_size;
            _image_rectangle = align(max_rectangle, new_image_size, hi::alignment::middle_center());
        }
    }

    [[nodiscard]] hi::quad_color fill_color() const noexcept
    {
        switch (*gradient) {
        case gradient_type::solid:
            return hi::quad_color(blue);
        case gradient_type::horizontal:
            return hi::quad_color{blue, red, blue, red};
        case gradient_type::vertical:
            return hi::quad_color{blue, blue, red, red};
        case gradient_type::corners:
            return hi::quad_color{red, blue, cyan, white};
        default:
            hi_no_default();
        }
    }

    [[nodiscard]] hi::quad_color line_color() const noexcept
    {
        if (*border_width == 0.0f) {
            // Due to inaccuracies in the shaders, a thin border may present itself inside
            // the anti-aliased edges; so make it the same color as the fill. This is the
            // same thing that happens when you call draw_box with only a fill color.
            return fill_color();
        } else {
            switch (*gradient) {
            case gradient_type::solid:
                return hi::quad_color(redish);
            case gradient_type::horizontal:
                return hi::quad_color{redish, greenish, redish, greenish};
            case gradient_type::vertical:
                return hi::quad_color{redish, redish, greenish, greenish};
            case gradient_type::corners:
                return hi::quad_color{redish, greenish, blueish, redish2};
            default:
                hi_no_default();
            }
        }
    }

    [[nodiscard]] hi::quad shape_quad() const noexcept
    {
        switch (*shape) {
        case shape_type::square:
            return hi::quad{
                hi::point3{-40.0f, -40.0f}, hi::point3{40.0f, -40.0f}, hi::point3{-40.0f, 40.0f}, hi::point3{40.0f, 40.0f}};
        case shape_type::rectangle:
            return hi::quad{
                hi::point3{-50.0f, -40.0f}, hi::point3{50.0f, -40.0f}, hi::point3{-50.0f, 40.0f}, hi::point3{50.0f, 40.0f}};
        case shape_type::convex:
            return hi::quad{
                hi::point3{-50.0f, -10.0f}, hi::point3{50.0f, -40.0f}, hi::point3{-50.0f, 40.0f}, hi::point3{50.0f, 50.0f}};
        case shape_type::concave:
            return hi::quad{
                hi::point3{20.0f, 20.0f}, hi::point3{50.0f, -40.0f}, hi::point3{-50.0f, 40.0f}, hi::point3{50.0f, 50.0f}};
        case shape_type::glyph_aspect_ratio:
            return _glyph_rectangle;
        case shape_type::image_aspect_ratio:
            return _image_rectangle;
        default:
            hi_no_default();
        }
    }

    [[nodiscard]] hi::rotate3 rotation(hi::draw_context const& context) const noexcept
    {
        float angle = 0.0f;
        if (*rotating) {
            request_redraw();
            constexpr auto interval_in_ns = 10'000'000'000;
            auto const repeating_interval = context.display_time_point.time_since_epoch().count() % interval_in_ns;
            angle = (float(repeating_interval) / float(interval_in_ns)) * 2.0f * std::numbers::pi_v<float>;
        }
        return hi::rotate3(angle, hi::vector3{0.0f, 0.0f, 1.0f});
    }

    [[nodiscard]] hi::corner_radii corners() const noexcept
    {
        if (*rounded) {
            return hi::corner_radii{20.0f, 10.0f, 5.0f, 0.0f};
        } else {
            return {};
        }
    }

    [[nodiscard]] hi::line_end_cap end_cap() const noexcept
    {
        return *rounded ? hi::line_end_cap::round : hi::line_end_cap::flat;
    }

    // The `draw()` function is called when all or part of the window requires redrawing.
    // This may happen when showing the window for the first time, when the operating-system
    // requests a (partial) redraw, or when a widget requests a redraw of itself.
    void draw(hi::draw_context const& context) noexcept override
    {
        using namespace std::chrono_literals;

        auto const clipping_rectangle =
            *clip ? hi::aarectangle{0.0f, 0.0f, _layout.width(), _layout.height() * 0.5f} : _layout.rectangle();

        auto const translation = hi::translate3(std::floor(_layout.width() * 0.5f), std::floor(_layout.height()) * 0.5f, 0.0f);
        auto const transform = translation * rotation(context);

        auto const circle = hi::circle{hi::point3{0.0f, 0.0f, 0.0f}, 50.0f};

        // We only need to draw the widget when it is visible and when the visible area of
        // the widget overlaps with the scissor-rectangle (partial redraw) of the drawing context.
        if (*mode > hi::widget_mode::invisible and overlaps(context, layout())) {
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

            case drawing_type::lines:
                {
                    // There is a concave corner at left-bottom, so I want this to be the second point the lines pass through.
                    auto const quad = shape_quad();
                    auto const line1 = hi::line_segment{get<0>(quad), get<1>(quad)};
                    auto const line2 = hi::line_segment{get<0>(quad), get<2>(quad)};
                    auto const line3 = hi::line_segment{get<3>(quad), get<2>(quad)};
                    auto const width = std::max(0.5f, *border_width);
                    context.draw_line(_layout, clipping_rectangle, transform * line1, width, fill_color(), end_cap(), end_cap());
                    context.draw_line(_layout, clipping_rectangle, transform * line2, width, fill_color(), end_cap(), end_cap());
                    context.draw_line(_layout, clipping_rectangle, transform * line3, width, fill_color(), end_cap(), end_cap());
                }
                break;

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

            default:
                hi_no_default();
            }
        }
    }

private:
    hi::glyph_ids _glyph;
    hi::aarectangle _glyph_rectangle;
    std::atomic<bool> _image_was_modified = true;
    hi::png _image;
    hi::aarectangle _image_rectangle;
    hi::paged_image _image_backing;

    decltype(drawing)::callback_token _drawing_cbt;
    decltype(shape)::callback_token _shape_cbt;
    decltype(gradient)::callback_token _gradient_cbt;
    decltype(rotating)::callback_token _rotating_cbt;
    decltype(clip)::callback_token _clip_cbt;
    decltype(border_side)::callback_token _border_side_cbt;
    decltype(border_width)::callback_token _border_width_cbt;
    decltype(rounded)::callback_token _rounded_cbt;
};

int hi_main(int argc, char *argv[])
{
    hi::observer<drawing_type> drawing = drawing_type::box;
    hi::observer<shape_type> shape = shape_type::square;
    hi::observer<bool> rotating = false;
    hi::observer<bool> clip = false;
    hi::observer<gradient_type> gradient = gradient_type::solid;
    hi::observer<hi::border_side> border_side = hi::border_side::on;
    hi::observer<float> border_width = 0.0f;
    hi::observer<bool> rounded = false;

    // Startup renderdoc for debugging
    auto render_doc = hi::RenderDoc();

    auto gui = hi::gui_system::make_unique();
    auto window = gui->make_window(hi::tr("Drawing Custom Widget"));

    auto& custom_widget = window->content().make_widget<drawing_widget>("A1:D1");
    custom_widget.drawing = drawing;
    custom_widget.shape = shape;
    custom_widget.rotating = rotating;
    custom_widget.clip = clip;
    custom_widget.gradient = gradient;
    custom_widget.border_side = border_side;
    custom_widget.border_width = border_width;
    custom_widget.rounded = rounded;

    window->content().make_widget<hi::label_widget>("A2", hi::tr("Drawing type:"));
    window->content().make_widget<hi::selection_widget>("B2:D2", drawing, drawing_list);

    window->content().make_widget<hi::label_widget>("A3", hi::tr("Shape:"));
    window->content().make_widget<hi::selection_widget>("B3:D3", shape, shape_list);

    window->content().make_widget<hi::label_widget>("A4", hi::tr("Gradient:"));
    window->content().make_widget<hi::selection_widget>("B4:D4", gradient, gradient_list);

    window->content().make_widget<hi::label_widget>("A5", hi::tr("Border side:"));
    window->content().make_widget<hi::radio_button_widget>("B5", border_side, hi::border_side::on, hi::tr("on"));
    window->content().make_widget<hi::radio_button_widget>("C5", border_side, hi::border_side::inside, hi::tr("inside"));
    window->content().make_widget<hi::radio_button_widget>("D5", border_side, hi::border_side::outside, hi::tr("outside"));

    window->content().make_widget<hi::label_widget>("A6", hi::tr("Border width:"));
    window->content().make_widget<hi::selection_widget>("B6:D6", border_width, border_width_list);

    window->content().make_widget<hi::label_widget>("A7", hi::tr("Rotate:"));
    window->content().make_widget<hi::toggle_widget>("B7:D7", rotating);

    window->content().make_widget<hi::label_widget>("A8", hi::tr("Clip:"));
    window->content().make_widget<hi::toggle_widget>("B8:D8", clip);

    window->content().make_widget<hi::label_widget>("A9", hi::tr("Rounded:"));
    window->content().make_widget<hi::toggle_widget>("B9:D9", rounded);

    auto close_cbt = window->closing.subscribe(
        [&] {
            window = {};
        },
        hi::callback_flags::main);
    return hi::loop::main().resume();
}
