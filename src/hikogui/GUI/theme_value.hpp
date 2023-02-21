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
#include "theme_value_index.hpp"
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

    [[nodiscard]] value_type get() const noexcept
    {
        hi_axiom(_mutex.is_locked());
        return _value;
    }

    [[nodiscard]] void set(value_type value) noexcept
    {
        hi_axiom(_mutex.is_locked());
        _value = value;
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
    std::array<value_type, theme_value_index::array_size> _values;

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

    [[nodiscard]] value_type get() const noexcept
    {
        return std::atomic_ref(_value).load(std::memory_order::relaxed);
    }

    [[nodiscard]] void set(value_type value) noexcept
    {
        std::atomic_ref(_value).store(value, std::memory_order::relaxed);
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
    value_type _value;

protected:
    inline static unfair_mutex _map_mutex;
    inline static std::map<std::string, theme_value_base *> _map;
};

class theme_value_base<hi::color> {
public:
    using value_type = hi::color;
    static_assert(std::atomic<value_type>::is_always_lock_free);

    constexpr theme_value_base() noexcept = default;
    theme_value_base(theme_value_base const&) = delete;
    theme_value_base(theme_value_base&&) = delete;
    theme_value_base& operator=(theme_value_base const&) = delete;
    theme_value_base& operator=(theme_value_base&&) = delete;

    void lock() noexcept {}

    void unlock() noexcept {}

    [[nodiscard]] value_type get(theme_value_index index) const noexcept
    {
        return std::atomic_ref(_values[index.intrinsic()]).load(std::memory_order::relaxed);
    }

    [[nodiscard]] void set(theme_value_index index, value_type value) noexcept
    {
        std::atomic_ref(_values[index.intrinsic()]).store(value, std::memory_order::relaxed);
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
    std::array<value_type, theme_value_index::array_size> _values;

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
        hilet lock = std::scoped_lock(value);
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
    [[nodiscard]] extent2i operator()(widget const *widget) const noexcept
    {
        hilet tmp = theme<Tag, int>{}(widget);
        return marginsi{tmp};
    }
};

template<fixed_string Tag>
struct theme<Tag, corner_radii> {
    [[nodiscard]] corner_radii operator()(widget const *widget) const noexcept
    {
        hilet tmp = theme<Tag, int>{}(widget);
        return corner_radii{tmp};
    }
};

template<fixed_string Tag>
struct theme<Tag, hi::color> {
    [[nodiscard]] hi::color operator()(widget const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        hilet& value = detail::global_theme_value<Tag, hi::color>;
        hilet lock = std::scoped_lock(value);
        return value.get(theme_value_index{*widget});
    }
};

template<fixed_string Tag>
struct theme<Tag, text_theme> {
    [[nodiscard]] text_theme operator()(widget const *widget) const noexcept
    {
        hi_axiom_not_null(widget);
        hilet& value = detail::global_theme_value<Tag, hi::text_theme>;
        hilet lock = std::scoped_lock(value);
        return value.get();
    }
};

}} // namespace hi::v1
