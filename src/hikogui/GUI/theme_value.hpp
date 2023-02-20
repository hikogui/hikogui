// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file theme_value.hpp Global variables for themese.
 */

#pragma once

#include "../concurrency/module.hpp"
#include "../utility/module.hpp"
#include "../color/module.hpp"
#include "../text/text_theme.hpp"
#include "../geometry/module.hpp"
#include "../generator.hpp"
#include <atomic>
#include <map>
#include <mutex>

namespace hi { inline namespace v1 {

namespace detail {
    

template<typename T>
class theme_value_base {
public:
    static_assert(not std::is_trivially_copyable_v<T>, "This should use the template specialization");

    using value_type = T;

    constexpr theme_value_base() noexcept = default;
    theme_value_base(theme_value_base const&) = delete;
    theme_value_base(theme_value_base&&) = delete;
    theme_value_base& operator=(theme_value_base const&) = delete;
    theme_value_base& operator=(theme_value_base&&) = delete;

    void lock() noexcept
    {
        return _mutex.lock();
    }

    void unlock() noexcept
    {
        return _mutex.unlock();
    }

    value_type const& operator*() const noexcept
    {
        hi_axiom(_mutex.is_locked());
        return _value;
    }

    value_type& operator*() noexcept
    {
        hi_axiom(_mutex.is_locked());
        return _value;
    }

    theme_value_base& operator=(value_type const& value) noexcept
    {
        hi_axiom(_mutex.is_locked());
        _value = value;
        return *this;
    }

    [[nodiscard]] static generator<theme_value_base&> get(std::string const& key) noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);

        for (hilet & [ item_key, item_value ] : _map) {
            if (pattern_match(key, item_key)) {
                co_yield *item_value;
            }
        }
    }

private:
    mutable unfair_mutex _mutex;
    value_type _value;

protected:
    inline static unfair_mutex _map_mutex;
    inline static std::map<std::string, theme_value_base *> _map;
};

template<trivially_copyable T>
    requires std::atomic<T>::is_always_lock_free
class theme_value_base<T> {
public:
    using value_type = T;

    constexpr theme_value_base() noexcept = default;
    theme_value_base(theme_value_base const&) = delete;
    theme_value_base(theme_value_base&&) = delete;
    theme_value_base& operator=(theme_value_base const&) = delete;
    theme_value_base& operator=(theme_value_base&&) = delete;

    void lock() noexcept {}

    void unlock() noexcept {}

    value_type operator*() const noexcept
    {
        return _value.load(std::memory_order::relaxed);
    }

    theme_value_base& operator=(value_type const& value) noexcept
    {
        _value.store(value, std::memory_order::relaxed);
        return *this;
    }

    [[nodiscard]] static generator<theme_value_base&> get(std::string const& key) noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);

        for (hilet & [ item_key, item_value ] : _map) {
            if (pattern_match(key, item_key)) {
                co_yield *item_value;
            }
        }
    }

private:
    std::atomic<value_type> _value;

protected:
    inline static unfair_mutex _map_mutex;
    inline static std::map<std::string, theme_value_base *> _map;
};



template<fixed_string Tag, typename T>
class tagged_theme_value : public theme_value_base<T> {
public:
    constexpr static auto tag = Tag;

    tagged_theme_value() noexcept : theme_value_base<T>()
    {
        hilet lock = std::scoped_lock(this->_map_mutex);
        this->_map[static_cast<std::string>(tag)] = this;
    }
};

template<fixed_string Tag, typename T>
inline auto global_theme_value = tagged_theme_value<Tag, T>{};

} // namespace detail

/** A functor to retrieve a theme value.
 *
 * There are different arguments for the function operator for each return type:
 *  - `float operator()(float dpi_scale)`
 *  - `int operator()(float dpi_scale)`
 *  - `color operator()(size_t depth)`
 *
 * @tparam Tag The name of the global theme value.
 * @tparam T The return type of the functor.
 */
template<fixed_string Tag, typename T>
struct tv;

template<fixed_string Tag, std::floating_point T>
struct tv<Tag, T> {
    [[nodiscard]] T operator()(float dpi_scale) const noexcept
    {
        hilet& value = detail::global_theme_value<Tag, float>;
        hilet lock = std::scoped_lock(value);
        return wide_cast<T>(*value * dpi_scale);
    }

    [[nodiscard]] T operator()(widget_impl const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        return (*this)(widget->semantic_layer);
    }
};

template<fixed_string Tag, std::integral T>
struct tv<Tag, T> {
    [[nodiscard]] T operator()(float dpi_scale) const noexcept
    {
        return narrow_cast<T>(std::ceil(tv<Tag, float>{}(dpi_scale)));
    }

    [[nodiscard]] T operator()(widget_impl const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        return (*this)(widget->semantic_layer);
    }
};

template<fixed_string Tag>
struct tv<Tag, extent2i> {
    [[nodiscard]] extent2i operator()(float dpi_scale) const noexcept
    {
        hilet tmp = tv<Tag, float>{}(dpi_scale);
        return extent2i{tmp, tmp};
    }

    [[nodiscard]] extent2i operator()(widget_impl const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        return (*this)(widget->semantic_layer);
    }
};

template<fixed_string Tag>
struct tv<Tag, marginsi> {
    [[nodiscard]] marginsi operator()(float dpi_scale) const noexcept
    {
        hilet tmp = tv<Tag, float>{}(dpi_scale);
        return marginsi{tmp};
    }

    template<derived_from<widget_impl> Widget> requires (not std::is_same_v<Widget, widget_impl>)
    [[nodiscard]] marginsi operator()(Widget const *widget) const noexcept
    {
        constexpr auto prefix = Widget::tag + "." + Tag;

        hi_axiom_not_null(widget);

        if (widget->mode <= widget_mode::disabled) {
            return tv<prefix + ":disabled", margins>{}(widget->dpi_scale);

        } else if (not widget->active) {
            return tv<prefix + ":passive", margins>{}(widget->dpi_scale);

        } else if (widget->focus) {
            if () {
            } else {
                if (widget->hover == widget_hover::pressed) {
                    return tv<prefix + ":focus:pressed", margins>{}(widget->dpi_scale);

                } else if (widget->hover = widget_hover::hover) {
                    return tv<prefix + ":focus:hover", margins>{}(widget->dpi_scale);

                } else {
                    return tv<prefix + ":focus", margins>{}(widget->dpi_scale);
                }
            }
        }


        return (*this)(widget->semantic_layer);
    }
};

template<fixed_string Tag>
struct tv<Tag, hi::color> {
    [[nodiscard]] hi::color operator()(size_t depth = 0) const noexcept
    {
        hilet& value = detail::global_theme_value<Tag, std::vector<hi::color>>;
        hilet lock = std::scoped_lock(value);
        hilet& colors = *value;
        if (colors.empty()) {
            return {};
        } else {
            return colors[depth % colors.size()];
        }
    }
};

template<fixed_string Tag>
struct tv<Tag, text_theme> {
    [[nodiscard]] text_theme operator()() const noexcept
    {
        hilet& value = detail::global_theme_value<Tag, text_theme>;
        hilet lock = std::scoped_lock(value);
        return *value;
    }
};


}} // namespace hi::v1
