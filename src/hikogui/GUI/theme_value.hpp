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
#include "../log.hpp"
#include "theme_value_index.hpp"
#include <atomic>
#include <map>
#include <mutex>

namespace hi { inline namespace v1 {

namespace detail {

template<typename T>
class theme_value_base {
public:
    using value_type = T;

    constexpr theme_value_base() noexcept = default;
    theme_value_base(theme_value_base const&) = delete;
    theme_value_base(theme_value_base&&) = delete;
    theme_value_base& operator=(theme_value_base const&) = delete;
    theme_value_base& operator=(theme_value_base&&) = delete;

    [[nodiscard]] value_type get() const noexcept
    {
        return _value;
    }

    [[nodiscard]] void set(value_type value) noexcept
    {
        _value = value;
        ++_count;
    }

    /** Find all the theme-values matching a key.
     *
     * @param key A key retrieved from the theme-config file.
     * @return A generator listing every value matching the key.
     */
    [[nodiscard]] static generator<theme_value_base&> find(std::string const& key) noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);

        for (hilet & [ item_key, item_ptr ] : _map) {
            if (pattern_match(key, item_key)) {
                co_yield *item_ptr;
            }
        }
    }

    /** Log all the theme-values.
     */
    static void log() noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);

        for (hilet & [ item_key, item_ptr ] : _map) {
            if (item_ptr->_count == 0) {
                hi_log_error(" * {} = unassigned", item_key);
            } else {
                hi_log_debug(" * {} = {}", item_key, item_ptr->_value);
            }
        }
    }

    /** Reset all the theme-values.
     */
    static void reset() noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);

        for (auto& [item_key, item_ptr] : _map) {
            item_ptr->_value = value_type{};
            item_ptr->_count = 0;
        }
    }

private:
    value_type _value = value_type{};

    /** The number of times a value has been assigned by the theme config file.
     */
    size_t _count = 0;

protected:
    inline static unfair_mutex _map_mutex;
    inline static std::map<std::string, theme_value_base *> _map;
};

template<>
class theme_value_base<hi::color> {
public:
    using value_type = hi::color;
    using color_array_type = std::array<value_type, theme_value_index::array_size>;

    constexpr theme_value_base() noexcept = default;
    theme_value_base(theme_value_base const&) = delete;
    theme_value_base(theme_value_base&&) = delete;
    theme_value_base& operator=(theme_value_base const&) = delete;
    theme_value_base& operator=(theme_value_base&&) = delete;

    [[nodiscard]] value_type get(theme_value_index index) const noexcept
    {
        return _values[index.intrinsic()];
    }

    [[nodiscard]] void set(theme_value_index index, value_type value) noexcept
    {
        _values[index.intrinsic()] = value;
    }

    [[nodiscard]] static generator<theme_value_base&> get(std::string const& key) noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);

        for (hilet & [ item_key, item_ptr ] : _map) {
            if (pattern_match(key, item_key)) {
                co_yield *item_ptr;
            }
        }
    }

    /** Log all the theme-values.
     */
    static void log() noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);

        for (hilet & [ item_key, item_ptr ] : _map) {
            if (item_ptr->_count == 0) {
                hi_log_error(" * {} = unassigned", item_key);
            } else {
                // Display each color separately.
                hi_log_debug(" * {}", item_key);
            }
        }
    }

    /** Reset all the theme-values.
     */
    static void reset() noexcept
    {
        hilet lock = std::scoped_lock(_map_mutex);

        for (auto& [item_key, item_ptr] : _map) {
            item_ptr->_values = color_array_type{};
            item_ptr->_count = 0;
        }
    }

private:
    color_array_type _values = {};

    /** The number of times a value has been assigned by the theme config file.
     */
    size_t _count = 0;

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
struct theme;

template<fixed_string Tag, std::floating_point T>
struct theme<Tag, T> {
    [[nodiscard]] T operator()(widget const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        hilet& value = detail::global_theme_value<Tag, float>;
        return wide_cast<T>(value.get() * widget->dpi_scale);
    }
};

template<fixed_string Tag, std::integral T>
struct theme<Tag, T> {
    [[nodiscard]] T operator()(widget const *widget) const noexcept
    {
        return narrow_cast<T>(std::ceil(theme<Tag, float>{}(widget)));
    }
};

template<fixed_string Tag>
struct theme<Tag, extent2i> {
    [[nodiscard]] extent2i operator()(widget const *widget) const noexcept
    {
        hilet tmp = theme<Tag, int>{}(widget);
        return extent2i{tmp, tmp};
    }
};

template<fixed_string Tag>
struct theme<Tag, marginsi> {
    [[nodiscard]] marginsi operator()(widget const *widget) const noexcept
    {
        hilet tmp = theme<Tag, int>{}(widget);
        return marginsi{tmp};
    }
};

template<fixed_string Tag>
struct theme<Tag, corner_radii> {
    [[nodiscard]] corner_radii operator()(widget const *widget) const noexcept
    {
        hilet tmp = theme<Tag, float>{}(widget);
        return corner_radii{tmp};
    }
};

template<fixed_string Tag>
struct theme<Tag, hi::color> {
    [[nodiscard]] hi::color operator()(widget const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        hilet& value = detail::global_theme_value<Tag, hi::color>;
        return value.get(theme_value_index{*widget});
    }
};

template<fixed_string Tag>
struct theme<Tag, text_theme> {
    [[nodiscard]] text_theme operator()(widget const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        hilet& value = detail::global_theme_value<Tag, hi::text_theme>;
        return value.get();
    }
};

}} // namespace hi::v1
