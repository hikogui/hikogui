// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "cast.hpp"
#include "assert.hpp"
#include <cstddef>
#include <type_traits>
#include <array>
#include <algorithm>

namespace tt::inline v1 {

/** A object that holds enum-values and strings.
 *
 * @tparam T The enum-type.
 * @tparam N Number of enum-values.
 */
template<typename T, size_t N>
class enum_metadata {
    static_assert(std::is_enum_v<T>, "Must be an enum");
    static_assert(N != 0);

public:
    using value_type = T;

    /** The number of enum values.
     */
    static constexpr size_t count = N;

    /** The numeric values in the enum do not contain a gap.
     */
    bool values_are_continues;

    /** Get the minimum value.
     */
    [[nodiscard]] constexpr value_type minimum() const noexcept
    {
        return std::get<0>(_by_value).value;
    }

    /** Get the maximum value.
     */
    [[nodiscard]] constexpr value_type maximum() const noexcept
    {
        return std::get<N-1>(_by_value).value;
    }

    /** Construct a enum-names table object.
     *
     * Example usage:
     * ```
     * enum class my_bool { yes, no };
     * constexpr auto my_bool_names = enum_metadata(my_bool::no, "no", my_bool::yes, "yes");
     * ```
     *
     * The template parameters of the class will be deduced from the
     * constructor. `N = sizeof...(Args) / 2`, `T = decltype(args[0])`.
     *
     * @param Args A list of a enum-value and names.
     */
    template<typename... Args>
    [[nodiscard]] constexpr enum_metadata(Args const &...args) noexcept
    {
        static_assert(sizeof...(Args) == N * 2);
        add_value_name<0>(args...);

        std::sort(_by_name.begin(), _by_name.end(), [](ttlet &a, ttlet &b) {
            return a.name < b.name;
        });

        std::sort(_by_value.begin(), _by_value.end(), [](ttlet &a, ttlet &b) {
            return to_underlying(a.value) < to_underlying(b.value);
        });

        values_are_continues = check_values_are_continues();
    }

    /** Check if the enum has a name.
     *
     * @param name The name to lookup in the enum.
     * @return True if the name is found.
     */
    [[nodiscard]] constexpr bool contains(std::string_view name) const noexcept
    {
        return find(name) != nullptr;
    }

    /** Check if the enum has a value.
     *
     * @param name The name to lookup in the enum.
     * @return True if the name is found.
     */
    [[nodiscard]] constexpr bool contains(value_type value) const noexcept
    {
        return find(value) != nullptr;
    }

    /** Get an enum-value from a name.
     *
     * @param name The name to lookup in the enum.
     * @return The enum-value belonging with the name.
     * @throws std::out_of_range When the name does not exist.
     */
    [[nodiscard]] constexpr value_type at(std::string_view name) const
    {
        if (ttlet *value = find(name)) {
            return *value;
        } else {
            throw std::out_of_range{"enum_metadata::at"};
        }
    }

    /** Get a name from an enum-value.
     *
     * @param value The enum value to lookup.
     * @return The name belonging with the enum value.
     * @throws std::out_of_range When the value does not exist.
     */
    [[nodiscard]] constexpr std::string_view const &at(value_type value) const
    {
        if (ttlet *name = find(value)) {
            return *name;
        } else {
            throw std::out_of_range{"enum_metadata::at"};
        }
    }

    /** Get an enum-value from a name.
     *
     * @param name The name to lookup in the enum.
     * @param default_value The default value to return when the name is not found.
     * @return The enum-value belonging with the name.
     */
    [[nodiscard]] constexpr value_type at(std::string_view name, value_type default_value) const noexcept
    {
        if (ttlet *value = find(name)) {
            return *value;
        } else {
            return default_value;
        }
    }

    /** Get a name from an enum-value.
     *
     * @param value The enum value to lookup.
     * @return The name belonging with the enum value.
     * @throws std::out_of_range When the value does not exist.
     */
    [[nodiscard]] constexpr std::string_view at(value_type value, std::string_view default_name) const noexcept
    {
        if (ttlet *name = find(value)) {
            return *name;
        } else {
            return default_name;
        }
    }

    /** Get an enum-value from a name.
     *
     * @note It is undefined-behavior to lookup a name that does not exist in the table.
     * @param name The name to lookup in the enum.
     * @return The enum-value belonging with the name.
     */
    [[nodiscard]] constexpr value_type operator[](std::string_view name) const noexcept
    {
        auto *value = find(name);
        tt_axiom(value != nullptr);
        return *value;
    }

    /** Get a name from an enum-value.
     *
     * @note It is undefined-behavior to lookup a value that does not exist in the table.
     * @param value The enum value to lookup.
     * @return The name belonging with the enum value.
     */
    [[nodiscard]] constexpr std::string_view const &operator[](value_type value) const noexcept
    {
        auto *name = find(value);
        tt_axiom(name != nullptr);
        return *name;
    }

private:
    struct value_name {
        value_type value;
        std::string_view name;
    };

    std::array<value_name, N> _by_name;
    std::array<value_name, N> _by_value;

    [[nodiscard]] constexpr std::string_view const *find(value_type value) const noexcept
    {
        if (values_are_continues) {
            // If the enum values are continues we can do an associative lookup.
            ttlet it = _by_value.begin();
            ttlet offset = to_underlying(it->value);
            ttlet i = to_underlying(value) - offset;
            return (i >= 0 and i < N) ? &(it + i)->name : nullptr;

        } else {
            ttlet it = std::lower_bound(_by_value.begin(), _by_value.end(), value, [](ttlet &item, ttlet &key) {
                return item.value < key;
            });

            return (it != _by_value.end() and it->value == value) ? &it->name : nullptr;
        }
    }

    [[nodiscard]] constexpr value_type const *find(std::string_view name) const noexcept
    {
        ttlet it = std::lower_bound(_by_name.begin(), _by_name.end(), name, [](ttlet &item, ttlet &key) {
            return item.name < key;
        });

        return (it != _by_name.end() and it->name == name) ? &it->value : nullptr;
    }

    /** Add value and name to table.
     *
     * Used by the constructor.
     */
    template<size_t I, typename... Rest>
    constexpr void add_value_name(value_type value, std::string_view name, Rest const &...rest) noexcept
    {
        static_assert(sizeof...(Rest) % 2 == 0);

        std::get<I>(_by_name) = {value, name};
        std::get<I>(_by_value) = {value, name};

        if constexpr (sizeof...(Rest) > 0) {
            add_value_name<I + 1>(rest...);
        }
    }

    /** Check if the values are continues
     *
     * Used by the constructor.
     */
    [[nodiscard]] constexpr bool check_values_are_continues() const noexcept
    {
        auto check_value = to_underlying(minimum());
        for (ttlet &item : _by_value) {
            if (to_underlying(item.value) != check_value++) {
                return false;
            }
        }
        return true;
    }
};

template<typename T, typename... Rest>
enum_metadata(T const &, Rest const &...) -> enum_metadata<T, (sizeof...(Rest) + 1) / 2>;

} // namespace tt
