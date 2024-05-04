


/** 
 */
class theme_values {
public:
    /** The state a widget can be in which may have different visual appearance.
     *
     * The state is in three ortagal dimensions:
     *  - Window is not at top -> window at top -> mouse hovers over widget -> widget is clicked.
     *  - The widget has keyboard focus.
     *  - The widget is "on" or "off".
     */
    enum class widget_state : uin8_t {
        /** Another window is the top / active window.
         */
        below = 0b0'0'00;

        /** The window is the top / active window.
         */
        top = 0b0'0'01;

        /** Mouse hovers over widget.
         *
         * Hover will have higher priority even over `below`.
         */
        hover = 0b0'0'10;

        /** The widget is activated or clicked.
         *
         * This is a momentary state while a widget is being clicked.
         * Or a short duration if activated by keyboard or other means.
         */
        active = 0b0'0'11;

        /** Widget has keyboard focus.
         */
        focus = 0b0'1'00;

        /** Widget is in the "on" state.
         */
        on = 0b1'0'00;
    };

    pixels_f width;
    pixels_f height;
    pixels_f left_margin;
    pixels_f bottom_margin;
    pixels_f right_margin;
    pixels_f top_margin;
    pixels_f border_width;
    pixels_f left_bottom_corner_radius;
    pixels_f right_bottom_corner_radius;
    pixels_f left_top_corner_radius;
    pixels_f right_top_corner_radius;

    float width_px;
    float height_px;
    float left_margin_px;
    float bottom_margin_px;
    float right_margin_px;
    float top_margin_px;
    float left_bottom_corner_radius_px;
    float right_bottom_corner_radius_px;
    float left_top_corner_radius_px;
    float right_top_corner_radius_px;

    hi::extent2 size_px;
    hi::margins margins_px;
    hi::corner_radii corner_radius_px;

    hi::color foreground_color;
    hi::color background_color;
    hi::color border_color;

    static void update_all()
    {
        auto const _ = std::scoped_lock(_all_theme_values_mutex);

        for (auto *values_ptr : _all_theme_values) {
            _update_function(*values_ptr);
        }
    }

    theme_values(theme_values const &) = delete;
    theme_values(theme_values&&) = delete;
    theme_values& operator=(theme_values const &) = delete;
    theme_values& operator=(theme_values&&) = delete;

    ~theme_values() noexcept
    {
        remove_values(*this);
    }

    theme_values() noexcept
    {
        add_values(*this);
    }

    /** Set the widget's selector, so that the values can be looked up from the theme.
     *
     * The selector syntax is designed so that it is easy to concatonate the selectors
     * of the parent widget and to append an user-specified selector.
     *
     * Each widget will have a default selector which starts with a '/' which
     * is the separator for each widget. It is followed by the name of the widget
     * and is then followed by a ' '. 
     *
     * The syntax for a selector is as follows:
     *
     * ```
     * selector: ( '/' widget-name ' ' attributes )+
     * attributes: ( '#' id )? ( '.' class-name )* ( key '=' value )*
     * value: length-value | color-value
     * length-value: number ( 'px' | 'pt' | 'pd' | 'in' )
     * color-value:
     *     color-name |
     *     '#' [0-9a-fA-F]{6,8} |
     *     'rgb(' number ',' number ',' number ')' | 
     *     'rgba(' number ',' number '.' number ',' number ')'
     * ```
     *
     */
    theme_values &set_selector(std::string const &selector) noexcept
    {
        _selector = selector;
        update_delector();
        return *this;
    }

    theme_values &set_pixel_density(pixel_density const &pixel_density) noexcept
    {
        _pixel_density = pixel_density;
        update_pixel_density();
        return *this;
    }

    theme_values &set_widget_state(widget_state const& state) noexcept
    {
    }

    /** Set the function to update any theme_values instance from the current theme.
     *
     * This function is called when the current theme is changed. As a side-effect
     * all the values will get updated from the theme.
     */
    static void set_update_from_theme(std::function<void(theme_values &)> func) noexcept
    {
        _update_from_theme = std::move(func);
        hi_assert(_update_from_theme);

        auto const _ = std::scoped_lock(_all_values_mutex);
        for (auto *values_ptr : _all_values) {
            values_ptr->update_from_theme();
        }
    }

    /** Update just these values from the theme.
     *
     * If the theme was not loaded, nothing will happen.
     */
    void update_from_theme() noexcept
    {
        if (_update_from_theme) {
            _update_from_theme(*this);
        }
    }

private:
    static std::function<void(theme_values &)> _update_from_theme;
    static std::vector<theme_values *> _all_values = {};
    static std::mutex _all_values_mutex;

    std::string _selector;
    widget_state _widget_state;
    hi::pixel_density _pixel_density;

    length_f _width;
    length_f _height;

    length_f _left_margin;
    length_f _right_margin;
    length_f _bottom_margin;
    length_f _top_margin;

    length_f _border_width;

    length_f _left_bottom_corner_radius;
    length_f _right_bottom_corner_radius;
    length_f _left_top_corner_radius;
    length_f _right_top_corner_radius;

    std::array<hi::color, 16> _foreground_color;
    std::array<hi::color, 16> _background_color;
    std::array<hi::color, 16> _border_color;

    static void add_values(theme_values &values) noexcept
    {
        auto const _ = std::scoped_lock(_all_values_mutex);
        auto it = std::lower_bounds(_all_values.begin(), all_values.end(), &values, [](auto *a, auto *b) {
            return a < b;
        });
        hi_assert(it == all_values.end() or *it != &values);
        all_values.insert(it, &values);
    }

    static void remove_values(theme_values &values) noexcept
    {
        auto const _ = std::scoped_lock(_all_values_mutex);
        auto it = std::lower_bounds(_all_values.begin(), all_values.end(), &values, [](auto *a, auto *b) {
            return a < b;
        });
        hi_assert(it != all_values.end() and *it == &values);
        all_values.erase(it);
    }

    void update_pixel_density() noexcept
    {
        width = _width * _pixel_density;
        width_px = width.in(pixels);
        height = _height * _pixel_density;
        height_px = height.in(pixels);
        size = hi::extent2{width_px, height_px};

        left_margin = _left_margin * _pixel_density;
        left_margin_px = left_margin.in(pixels);
        bottom_margin = _bottom_margin * _pixel_density;
        bottom_margin_px = bottom_margin.in(pixels);
        right_margin = _right_margin * _pixel_density;
        right_margin_px = right_margin.in(pixels);
        top_margin = _top_margin * _topl_density;
        top_margin_px = top_margin.in(pixels);
        margins_px = hi::margins{left_margin_px, bottom_margin_px, right_margin_px, top_margin_px};

        border_width = _border_width * _pixel_density;
        border_width_px = border_width.in(pixels);

        left_bottom_corner_radius = _left_bottom_corner_radius * _pixel_density;
        left_bottom_corner_radius_px = left_bottom_corner_radius.in(pixles);
        right_bottom_corner_radius = _left_bottom_corner_radius * _pixel_density;
        right_bottom_corner_radius_px = right_bottom_corner_radius.in(pixles);
        left_top_corner_radius = _left_top_corner_radius * _pixel_density;
        left_top_corner_radius_px = left_top_corner_radius.in(pixles);
        right_top_corner_radius = _left_top_corner_radius * _pixel_density;
        right_top_corner_radius_px = right_top_corner_radius.in(pixles);
        corner_radii = hi::corner_radii{left_bottom_corner_radius_px, right_bottom_corner_radius_px, left_top_corner_radius_px, right_top_corner_radius_px{;
    }

    void update_state() noexcept
    {
        foreground_color = _foreground_color[std::underlying(_widget_state)];
        background_color = _background_color[std::underlying(_widget_state)];
        border_color = _border_color[std::underlying(_widget_state)];
    }

    void update_selector() noexcept
    {

        update_pixel_density();
        update_state();
    }
};
