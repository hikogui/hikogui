
#pragma once

hi_export_module(hikogui.theme.theme_variable);

hi_export namespace hi { inline namespace v1 {



template<typename T, fixed_string Tag>
class theme_variable {
public:
    using value_type = T;
    constexpr static auto tag = Tag;

    /** Set the value for all variables matching a selector.
     *
     * @param needle A theme-selector for searching variables.
     * @param value The value to set.
     */
    static void set(theme_selector const& needle, value_type const &value) noexcept
    {
        for (auto *ptr : _values) {
            if (match(needle, ptr->_selector)) {
                ptr->_v = value;
            }
        }
    }

    theme_variable(theme_variable const&) = delete;
    theme_variable(theme_variable&&) = delete;
    theme_variable& operator=(theme_variable const&) = delete;
    theme_variable& operator=(theme_variable&&) = delete;

    ~theme_variable()
    {
        auto const it = std::lower_bound(_values.begin(), _values.end(), this);
        hi_axiom(it != _values.end());
        _values.erase(it);
    }

    theme_variable(theme_selector const& selector) noexcept : _selector(selector)
    {
        auto const it = std::lower_bound(_values.begin(), _values.end(), this);
        hi_axiom(it == _values.end() or *it != this);
        _values.insert(it, this);
    }

    [[nodiscard]] value_type const& operator*() const noexcept
    {
        return _v;
    }

    template<typename... Args>
    [[nodiscard]] decltype(auto) operator()(Args const&... args) const noexcept
    {
        return _v(args...);
    }

    template<typename Arg>
    [[nodiscard]] decltype(auto) operator[](Arg const& arg) const noexcept
    {
        return _v[arg];
    }

private:
    theme_selector _selector;
    value_type _v;

    inline static std::vector<theme_variable> _values;
};


using margin_left = theme_variable<"margin-left", theme_length>;
using margin_right = theme_variable<"margin-right", theme_length>;
using margin_top = theme_variable<"margin-top", theme_length>;
using margin_bottom = theme_variable<"margin-bottom", theme_length>;
using margin = theme_variable<"margin", theme_length_quad>;

}}

