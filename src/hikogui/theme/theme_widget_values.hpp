


/** 
 */
class theme_widget_values {
public:
    enum class widget_state : uin8_t {

    };

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
    theme_widget_values &set_selector(std::string const &selector) noexcept
    {
        _selector = selector;
        update_delector();
        return *this;
    }

    theme_widget_values &set_pixel_density(pixel_density const &pixel_density) noexcept
    {
        _pixel_density = pixel_density;
        update_pixel_density();
        return *this;
    }

    theme_widget_values &set_widget_state(widget_state const& state) noexcept
    {
    }

private:
    std::string _selector;
    hi::pixel_density _pixel_desnity;


    void update_pixel_density() noexcept
    {
    }

    void update_selector() noexcept
    {
    }
};
