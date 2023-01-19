// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "cast.hpp"
#include <cstddef>
#include <type_traits>
#include <array>
#include <algorithm>
#include <string_view>

hi_warning_push();
// C26445: Do not assign gsl::span or std::string_view to a reference. They are cheap to construct and are not owners of
// the underlying data. (gsl.view).
// False positive, sometimes the template is instantiated with string_view, sometimes not.
hi_warning_ignore_msvc(26445);

namespace hi::inline v1 {

/** A object that holds enum-values and strings.
 *
 * @tparam ValueType The enum-type.
 * @tparam NameType The type used to convert to and from the EnumType.
 * @tparam N Number of enum-values.
 */
template<typename ValueType, typename NameType, std::size_t N>
class enum_metadata {
public:
    using value_type = ValueType;
    using name_type = NameType;

    /** The number of enum values.
     */
    static constexpr std::size_t count = N;

    /** The numeric values in the enum do not contain a gap.
     */
    bool values_are_continues;

    static_assert(std::is_enum_v<value_type>, "value_type Must be an enum");
    static_assert(N != 0);

    /** Get the number of enum values.
     */
    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return count;
    }

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
        return std::get<N - 1>(_by_value).value;
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
     * @param args A list of a enum-value and names.
     */
    template<typename... Args>
    [[nodiscard]] constexpr enum_metadata(Args const&...args) noexcept
    {
        static_assert(sizeof...(Args) == N * 2);
        add_value_name<0>(args...);

        std::sort(_by_name.begin(), _by_name.end(), [](hilet& a, hilet& b) {
            return a.name < b.name;
        });

        std::sort(_by_value.begin(), _by_value.end(), [](hilet& a, hilet& b) {
            return to_underlying(a.value) < to_underlying(b.value);
        });

        values_are_continues = check_values_are_continues();
    }

    /** Check if the enum has a name.
     *
     * @param name The name to lookup in the enum.
     * @return True if the name is found.
     */
    [[nodiscard]] constexpr bool contains(std::convertible_to<name_type> auto&& name) const noexcept
    {
        return find(name_type{hi_forward(name)}) != nullptr;
    }

    /** Check if the enum has a value.
     *
     * @param value The value to lookup for the enum.
     * @return True if the value is found.
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
    [[nodiscard]] constexpr value_type at(std::convertible_to<name_type> auto&& name) const
    {
        if (hilet *value = find(name_type{hi_forward(name)})) {
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
    [[nodiscard]] constexpr name_type const& at(value_type value) const
    {
        if (hilet *name = find(value)) {
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
    [[nodiscard]] constexpr value_type at(std::convertible_to<name_type> auto&& name, value_type default_value) const noexcept
    {
        if (hilet *value = find(name_type{hi_forward(name)})) {
            return *value;
        } else {
            return default_value;
        }
    }

    /** Get a name from an enum-value.
     *
     * @param value The enum value to lookup.
     * @param default_name The default name to return when value is not found.
     * @return The name belonging with the enum value.
     */
    [[nodiscard]] constexpr name_type at(value_type value, std::convertible_to<name_type> auto&& default_name) const noexcept
    {
        if (hilet *name = find(value)) {
            return *name;
        } else {
            return hi_forward(default_name);
        }
    }

    /** Get an enum-value from a name.
     *
     * @note It is undefined-behavior to lookup a name that does not exist in the table.
     * @param name The name to lookup in the enum.
     * @return The enum-value belonging with the name.
     */
    [[nodiscard]] constexpr value_type operator[](std::convertible_to<name_type> auto&& name) const noexcept
    {
        auto *value = find(name_type{hi_forward(name)});
        hi_assert_not_null(value);
        return *value;
    }

    /** Get a name from an enum-value.
     *
     * @note It is undefined-behavior to lookup a value that does not exist in the table.
     * @param value The enum value to lookup.
     * @return The name belonging with the enum value.
     */
    [[nodiscard]] constexpr name_type const& operator[](value_type value) const noexcept
    {
        auto *name = find(value);
        hi_assert_not_null(name);
        return *name;
    }

private:
    struct value_name {
        value_type value;
        name_type name;

        constexpr value_name() noexcept : value(), name() {}
        constexpr value_name(value_type value, name_type name) noexcept : value(value), name(std::move(name)) {}
    };

    std::array<value_name, N> _by_name;
    std::array<value_name, N> _by_value;

    [[nodiscard]] constexpr name_type const *find(value_type value) const noexcept
    {
        if (values_are_continues) {
            // If the enum values are continues we can do an associative lookup.
            hilet it = _by_value.begin();
            hilet offset = to_underlying(it->value);
            hilet i = to_underlying(value) - offset;
            return (i >= 0 and i < N) ? &(it + i)->name : nullptr;

        } else {
            hilet it = std::lower_bound(_by_value.begin(), _by_value.end(), value, [](hilet& item, hilet& key) {
                return item.value < key;
            });

            return (it != _by_value.end() and it->value == value) ? &it->name : nullptr;
        }
    }

    [[nodiscard]] constexpr value_type const *find(name_type const& name) const noexcept
    {
        hilet it = std::lower_bound(_by_name.begin(), _by_name.end(), name, [](hilet& item, hilet& key) {
            return item.name < key;
        });

        return (it != _by_name.end() and it->name == name) ? &it->value : nullptr;
    }

    /** Add value and name to table.
     *
     * Used by the constructor.
     */
    template<std::size_t I, typename... Rest>
    constexpr void add_value_name(value_type value, name_type name, Rest const&...rest) noexcept
    {
        static_assert(sizeof...(Rest) % 2 == 0);

        std::get<I>(_by_name) = {value, name};
        std::get<I>(_by_value) = {value, std::move(name)};

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
        for (hilet& item : _by_value) {
            if (to_underlying(item.value) != check_value++) {
                return false;
            }
        }
        return true;
    }
};

template<typename T>
struct enum_metadata_name {
    using type = std::decay_t<T>;
};

// clang-format off
template<> struct enum_metadata_name<char const *> { using type = std::string_view; };
template<> struct enum_metadata_name<char *> { using type = std::string_view; };
template<size_t N> struct enum_metadata_name<char [N]> { using type = std::string_view; };
template<size_t N> struct enum_metadata_name<char const [N]> { using type = std::string_view; };
// clang-format on

template<typename T>
using enum_metadata_name_t = enum_metadata_name<T>::type;

template<typename ValueType, typename NameType, typename... Rest>
enum_metadata(ValueType const&, NameType const&, Rest const&...)
    -> enum_metadata<ValueType, enum_metadata_name_t<NameType>, (sizeof...(Rest) + 2) / 2>;

} // namespace hi::inline v1

hi_warning_pop();
