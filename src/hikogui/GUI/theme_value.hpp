// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file theme_value.hpp Global variables for themese.
 */

#pragma once

#include "../concurrency/module.hpp"
#include "../utility/module.hpp"
#include "../generator.hpp"
#include <atomic>
#include <map>
#include <mutex>

namespace hi { inline namespace v1 {

namespace detail {

template<typename T>
class theme_value_base {
public:
    using value_type = T;
    constexpr static auto is_lock_free = std::atomic<value_type>::is_always_lock_free;

    constexpr theme_value_base() noexcept = default;
    theme_value_base(theme_value_base const&) = delete;
    theme_value_base(theme_value_base&&) = delete;
    theme_value_base& operator=(theme_value_base const&) = delete;
    theme_value_base& operator=(theme_value_base&&) = delete;

    void lock() noexcept
    {
        if constexpr (not is_lock_free) {
            return _mutex.lock();
        }
    }

    void unlock() noexcept
    {
        if constexpr (not is_lock_free) {
            return _mutex.unlock();
        }
    }

    value_type operator*() const noexcept
        requires(is_lock_free)
    {
        return std::atomic_ref(_value).load(std::memory_order::relaxed);
    }

    value_type const& operator*() const noexcept
        requires(not is_lock_free)
    {
        hi_axiom(_mutex.is_locked());
        return _value;
    }

    value_type& operator*() noexcept
        requires(not is_lock_free)
    {
        hi_axiom(_mutex.is_locked());
        return _value;
    }

    theme_value_base& operator=(value_type const& value) noexcept
    {
        if constexpr (is_lock_free) {
            std::atomic_ref(_value).store(value, std::memory_order::relaxed);
        } else {
            hi_axiom(_mutex.is_locked());
            _value = value;
        }
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

template<fixed_string Tag, typename T>
class tagged_theme_value : theme_value_base<T> {
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

template<fixed_string Tag>
struct tv<Tag, float> {
    [[nodiscard]] float operator()(float dpi_scale) const noexcept
    {
        hilet& value = detail::global_theme_value<Tag, float>;
        hilet lock = std::scoped_lock(value);
        return *value * dpi_scale;
    }
};

template<fixed_string Tag>
struct tv<Tag, int> {
    [[nodiscard]] int operator()(float dpi_scale) const noexcept
    {
        return narrow_cast<int>(std::ceil(tv<Tag, float>(dpi_scale)));
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
struct tv<Tag, text_theme_id> {
    [[nodiscard]] float operator()() const noexcept
    {
        hilet& value = detail::global_theme_value<Tag, text_theme_id>;
        hilet lock = std::scoped_lock(value);
        return *value;
    }
};

}} // namespace hi::v1
