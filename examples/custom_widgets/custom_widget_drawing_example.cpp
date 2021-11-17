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
#include "ttauri/text/font_book.hpp"
#include "ttauri/GFX/RenderDoc.hpp"
#include "ttauri/crt.hpp"
#include <numbers>

enum class drawing_type {
    solid_rectangle,
    clipped_solid_rectangle,
    bordered_rectangle,
    clipped_bordered_rectangle,
    solid_rounded,
    clipped_solid_rounded,
    bordered_rounded,
    clipped_bordered_rounded,
    strange_shape,
    clipped_strange_shape,
    solid_circle,
    clipped_solid_circle,
    bordered_circle,
    clipped_bordered_circle,
    multicolor_circle,
    glyph,
    clipped_glyph,
    multicolor_glyph,
    clipped_multicolor_glyph,
};

drawing_type &operator++(drawing_type &rhs) noexcept
{
    if (rhs != drawing_type::clipped_multicolor_glyph) {
        rhs = static_cast<drawing_type>(tt::to_underlying(rhs) + 1);
    }
    return rhs;
}

drawing_type &operator--(drawing_type &rhs) noexcept
{
    if (rhs != drawing_type::solid_rectangle) {
        rhs = static_cast<drawing_type>(tt::to_underlying(rhs) - 1);
    }
    return rhs;
}

auto drawing_list = std::vector<std::pair<drawing_type, tt::label>>{
    {drawing_type::solid_rectangle, tt::l10n("Solid rectangle")},
    {drawing_type::clipped_solid_rectangle, tt::l10n("Clipped solid rectangle")},
    {drawing_type::bordered_rectangle, tt::l10n("Bordered rectangle")},
    {drawing_type::clipped_bordered_rectangle, tt::l10n("Clipped bordered rectangle")},
    {drawing_type::solid_rounded, tt::l10n("Solid rounded rectangle")},
    {drawing_type::clipped_solid_rounded, tt::l10n("Clipped solid rounded rectangle")},
    {drawing_type::bordered_rounded, tt::l10n("Bordered rounded rectangle")},
    {drawing_type::clipped_bordered_rounded, tt::l10n("Clipped bordered rounded rectangle")},
    {drawing_type::strange_shape, tt::l10n("Strange shape")},
    {drawing_type::clipped_strange_shape, tt::l10n("Clipped strange shape")},
    {drawing_type::solid_circle, tt::l10n("Solid circle")},
    {drawing_type::clipped_solid_circle, tt::l10n("Clipped solid circle")},
    {drawing_type::bordered_circle, tt::l10n("Bordered circle")},
    {drawing_type::clipped_bordered_circle, tt::l10n("Clipped bordered circle")},
    {drawing_type::multicolor_circle, tt::l10n("Mulicolor circle")},
    {drawing_type::glyph, tt::l10n("glyph")},
    {drawing_type::clipped_glyph, tt::l10n("Clipped glyph")},
    {drawing_type::multicolor_glyph, tt::l10n("Strange glyph")},
    {drawing_type::clipped_multicolor_glyph, tt::l10n("Clipped strange glyph")},

};

// Every widget must inherit from tt::widget.
class drawing_widget : public tt::widget {
public:
    tt::observable<drawing_type> selected_drawing = drawing_type::solid_rectangle;
    tt::observable<bool> rotating = false;

    // Every constructor of a widget starts with a `window` and `parent` argument.
    // In most cases these are automatically filled in when calling a container widget's `make_widget()` function.
    template<typename SelectedDrawing>
    drawing_widget(tt::gui_window &window, tt::widget *parent, SelectedDrawing &&selected_drawing) noexcept :
        widget(window, parent), selected_drawing(std::forward<SelectedDrawing>(selected_drawing))
    {
        this->selected_drawing.subscribe(_redraw_callback);
        this->rotating.subscribe(_redraw_callback);
        this->_glyph = font_book().find_glyph(tt::elusive_icon::Briefcase);
    }

    // The set_constraints() function is called when the window is first initialized,
    // or when a widget wants to change its constraints.
    tt::widget_constraints const &set_constraints() noexcept override
    {
        // Almost all widgets will reset the `_layout` variable here so that it will
        // trigger the calculations in `set_layout()` as well.
        _layout = {};

        // Certain expensive calculations, such as loading of images and shaping of text
        // can be done in this function.

        // The constrains below have different minimum, preferred and maximum sizes.
        // When the window is initially created it will try to size itself so that
        // the contained widgets are at their preferred size. Having a different minimum
        // and/or maximum size will allow the window to be resizable.
        return _constraints = {{100, 100}, {150, 150}, {200, 200}, theme().margin};
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
            // Here we can do some semi-expensive calculations which must be done when resizing the widget.
            // In this case we make two rectangles which are used in the `draw()` function.
            _clip_rectangle = tt::aarectangle{0.0f, 0.0f, layout.width() * 0.5f, layout.height() * 0.5f};
        }
    }

    // The `draw()` function is called when all or part of the window requires redrawing.
    // This may happen when showing the window for the first time, when the operating-system
    // requests a (partial) redraw, or when a widget requests a redraw of itself.
    void draw(tt::draw_context const &context) noexcept override
    {
        using namespace std::chrono_literals;

        if (rotating) {
            request_redraw();
            constexpr auto interval_in_ns = 10'000'000'000;
            ttlet repeating_interval = context.display_time_point.time_since_epoch().count() % interval_in_ns;
            _angle = (float(repeating_interval) / float(interval_in_ns)) * 2.0f * std::numbers::pi_v<float>;
        } else {
            _angle = 0.0f;
        }

        ttlet rotation = tt::rotate3(_angle, tt::vector3{0.0f, 0.0f, 1.0f});
        ttlet translation = tt::translate3(std::floor(_layout.width() * 0.5f), std::floor(_layout.height()) * 0.5f, 0.0f);
        ttlet transform = translation * rotation;

        ttlet rectangle =
            tt::quad{tt::point3{-50.0f, -40.0f}, tt::point3{50.0f, -40.0f}, tt::point3{-50.0f, 40.0f}, tt::point3{50.0f, 40.0f}};

        ttlet strange =
            tt::quad{tt::point3{-50.0f, -10.0f}, tt::point3{50.0f, -40.0f}, tt::point3{-50.0f, 40.0f}, tt::point3{50.0f, 50.0f}};

        ttlet circle = tt::circle{tt::point3{0.0f, 0.0f, 0.0f}, 50.0f};

        ttlet fill_color = tt::color(0.05f, 0.05f, 0.50f);
        ttlet line_color = tt::color(0.70f, 0.30f, 0.00f);

        ttlet fill_color_quad = tt::quad_color{
            tt::color(0.05f, 0.05f, 0.50f),
            tt::color(0.50f, 0.05f, 0.05f),
            tt::color(0.05f, 0.50f, 0.50f),
            tt::color(0.50f, 0.50f, 0.50f)};

        ttlet line_color_quad = tt::quad_color{
            tt::color(0.70f, 0.30f, 0.00f),
            tt::color(0.00f, 0.30f, 0.70f),
            tt::color(0.00f, 0.70f, 0.30f),
            tt::color(0.70f, 0.00f, 0.30f)};

        ttlet corners = tt::corner_shapes{20.0f, 10.0f, 5.0f, 0.0f};

        // We only need to draw the widget when it is visible and when the visible area of
        // the widget overlaps with the scissor-rectangle (partial redraw) of the drawing context.
        if (visible and overlaps(context, layout())) {
            switch (*selected_drawing) {
            case drawing_type::solid_rectangle:
                // A full rectangle is visible.
                context.draw_box(_layout, transform * rectangle, fill_color);
                break;

            case drawing_type::clipped_solid_rectangle:
                // Only the lower left corner of the rectangle is within the clipping rectangle.
                context.draw_box(_layout, _clip_rectangle, transform * rectangle, fill_color);
                break;

            case drawing_type::bordered_rectangle:
                // A full rectangle is visible.
                context.draw_box(_layout, transform * rectangle, fill_color, line_color, 2.0f, tt::border_side::inside);
                break;

            case drawing_type::clipped_bordered_rectangle:
                // A full rectangle is visible.
                context.draw_box(
                    _layout, _clip_rectangle, transform * rectangle, fill_color, line_color, 2.0f, tt::border_side::inside);
                break;

            case drawing_type::solid_rounded:
                // A full rectangle is visible.
                context.draw_box(_layout, transform * rectangle, fill_color, corners);
                break;

            case drawing_type::clipped_solid_rounded:
                // Only the lower left corner of the rectangle is within the clipping rectangle.
                context.draw_box(_layout, _clip_rectangle, transform * rectangle, fill_color, corners);
                break;

            case drawing_type::bordered_rounded:
                // A full rectangle is visible.
                context.draw_box(_layout, transform * rectangle, fill_color, line_color, 2.0f, tt::border_side::inside, corners);
                break;

            case drawing_type::clipped_bordered_rounded:
                // A full rectangle is visible.
                context.draw_box(
                    _layout,
                    _clip_rectangle,
                    transform * rectangle,
                    fill_color,
                    line_color,
                    2.0f,
                    tt::border_side::inside,
                    corners);
                break;

            case drawing_type::strange_shape:
                // A full rectangle is visible.
                context.draw_box(
                    _layout, transform * strange, fill_color_quad, line_color_quad, 2.0f, tt::border_side::inside, corners);
                break;

            case drawing_type::clipped_strange_shape:
                // A full rectangle is visible.
                context.draw_box(
                    _layout,
                    _clip_rectangle,
                    transform * strange,
                    fill_color_quad,
                    line_color_quad,
                    2.0f,
                    tt::border_side::inside,
                    corners);
                break;

            case drawing_type::solid_circle:
                // A full rectangle is visible.
                context.draw_circle(_layout, translation * circle, fill_color);
                break;

            case drawing_type::clipped_solid_circle:
                // Only the lower left corner of the rectangle is within the clipping rectangle.
                context.draw_circle(_layout, _clip_rectangle, translation * circle, fill_color);
                break;

            case drawing_type::bordered_circle:
                // A full rectangle is visible.
                context.draw_circle(_layout, translation * circle, fill_color, line_color, 2.0f, tt::border_side::inside);
                break;

            case drawing_type::clipped_bordered_circle:
                // A full rectangle is visible.
                context.draw_circle(
                    _layout, _clip_rectangle, translation * circle, fill_color, line_color, 2.0f, tt::border_side::inside);
                break;

            case drawing_type::multicolor_circle:
                // A full rectangle is visible.
                context.draw_circle(_layout, translation * circle, fill_color_quad, line_color_quad, 2.0f, tt::border_side::inside);
                break;

            case drawing_type::glyph:
                // A full rectangle is visible.
                context.draw_glyph(_layout, transform * rectangle, fill_color, _glyph);
                break;

            case drawing_type::clipped_glyph:
                // A full rectangle is visible.
                context.draw_glyph(_layout, _clip_rectangle, transform * rectangle, fill_color, _glyph);
                break;

            case drawing_type::multicolor_glyph:
                // A full rectangle is visible.
                context.draw_glyph(_layout, transform * rectangle, fill_color_quad, _glyph);
                break;

            case drawing_type::clipped_multicolor_glyph:
                // A full rectangle is visible.
                context.draw_glyph(_layout, _clip_rectangle, transform * rectangle, fill_color_quad, _glyph);
                break;
            }
        }
    }

private:
    float _angle = 0.0f;
    tt::aarectangle _clip_rectangle;
    tt::font_glyph_ids _glyph;
};

int tt_main(int argc, char *argv[])
{
    tt::observable<drawing_type> selected_drawing = drawing_type::solid_rectangle;
    tt::observable<bool> rotating;

    // Startup renderdoc for debugging
    auto render_doc = tt::RenderDoc();

    auto gui = tt::gui_system::make_unique();
    auto &window = gui->make_window(tt::l10n("Drawing Custom Widget"));

    auto &drawing = window.content().make_widget<drawing_widget>("A1", selected_drawing);
    drawing.rotating = rotating;

    auto &grid = window.content().make_widget<tt::grid_widget>("A2");

    grid.make_widget<tt::label_widget>("A1", tt::l10n("Drawing:"));
    grid.make_widget<tt::selection_widget>("B1", drawing_list, selected_drawing);
    auto &row = grid.make_widget<tt::row_widget>("B2");

    auto &prev_button = row.make_widget<tt::momentary_button_widget>(tt::l10n("previous"));
    auto prev_cbptr = prev_button.subscribe([&selected_drawing]() {
        --selected_drawing;
    });

    auto &next_button = row.make_widget<tt::momentary_button_widget>(tt::l10n("next"));
    auto next_cbptr = next_button.subscribe([&selected_drawing]() {
        ++selected_drawing;
    });

    grid.make_widget<tt::label_widget>("A3", tt::l10n("Rotate:"));
    grid.make_widget<tt::toggle_widget>("B3", rotating);

    return gui->loop();
}
