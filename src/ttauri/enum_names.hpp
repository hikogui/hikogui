

#pragma once

namespace tt {

/** A object that holds enum-values and strings.
 *
 * @tparam T The enum-type.
 * @tparam N Number of enum-values.
 */
template<typename T, size_t N>
class enum_names {
public:
    using value_type = T;
    static_assert(std::is_enum_v<T>, "Must be an enum");
    static_assert(N != 0);

    /** Construct a enum-names table object.
     *
     * Example usage:
     * ```
     * enum class my_bool { yes, no };
     * constexpr auto my_bool_names = enum_names(my_bool::no, "no", my_bool::yes, "yes");
     * ```
     *
     * The template parameters of the class will be deduced from the
     * constructor. `N = sizeof...(Args) / 2`, `T = decltype(args[0])`.
     *
     * @param Args A list of a enum-value and names.
     */
    template<typename... Args>
    [[nodiscard]] constexpr enum_names(Args const &... args) noexcept
    {
        static_assert(sizeof...(Args) == N * 2);
        add_value_name<0>(args...);

        std::sort(_by_name.begin(), _by_name.end(), [](ttlet &a, ttlet &b) {
            return a.name < b.name;
        });

        std::sort(_by_value.begin(), _by_value.end(), [](ttlet &a, ttlet &b) {
            return to_underlying(a.value) < to_underlying(b.value);
        });

        _values_are_continues = check_values_are_continues();
    }

    /** Check if the enum has a name.
     *
     * @param name The name to lookup in the enum.
     * @return True if the name is found.
     */
    [[nodiscard]] constexpr bool contains(std::string_view name)
    {
        ttlet it = std::lower_bound(_by_name.begin(), _by_name.end(), name);
        return it != _by_name.end() and it->name == name;
    }

    /** Get an enum-value from a name.
     *
     * @param name The name to lookup in the enum.
     * @return The enum-value beloging with the name.
     * @throws std::out_of_range When the name does not exist.
     */
    [[nodiscard]] constexpr value_type at(std::string_view name)
    {
        ttlet it = std::lower_bound(_by_name.begin(), _by_name.end(), name);
        if (it == _by_name.end() or it->name != name) {
            throw std::out_of_range();
        }
        return it->value;
    }

    /** Get a name from an enum-value.
     *
     * @param value The enum value to lookup.
     * @return The name belonging with the enum value.
     * @throws std::out_of_range When the value does not exist.
     */
    [[nodiscard]] constexpr std::string_view at(value_type value) noexcept
    {
        if (_values_are_continues) {
            // If the enum values are continues we can do an associative lookup.
            ttlet it = _by_value.begin();
            ttlet offset = to_underlying(it->value);
            ttlet i = to_underlying(value) - offset;
            if (i >= 0 and i < N) {
                return (it + i)->name;
            } else {
                throw std::out_of_range();
            }

        } else {
            ttlet it = std::lower_bound(_by_value.begin(), _by_value.end(), value);
            if (it == _by_value.end or it->value != value) {
                throw std::out_of_range();
            }
            return it->name;
        }
    }

    /** Get an enum-value from a name.
     *
     * @note It is undefined-behaviour to lookup a name that does not exist in the table.
     * @param name The name to lookup in the enum.
     * @return The enum-value beloging with the name.
     */
    [[nodiscard]] constexpr value_type operator[](std::string_view name) noexcept
    {
        ttlet it = std::lower_bound(_by_name.begin(), _by_name.end(), name);
        tt_axiom(it != _by_name.end() and it->name == name);
        return it->value;
    }

    /** Get a name from an enum-value.
     *
     * @note It is undefined-behaviour to lookup a value that does not exist in the table.
     * @param value The enum value to lookup.
     * @return The name belonging with the enum value.
     */
    [[nodiscard]] constexpr std::string_view operator[](value_type value) noexcept
    {
        if (_values_are_continues) {
            // If the enum values are continues we can do an associative lookup.
            ttlet it = _by_value.begin();
            ttlet offset = to_underlying(it->value);
            ttlet i = to_underlying(value) - offset;
            tt_axiom(i >= 0 and i < N);
            return (it + i)->name;

        } else {
            ttlet it = std::lower_bound(_by_value.begin(), _by_value.end(), value);
            tt_axiom(it != _by_value.end() and it->value == value);
            return it->name;
        }
    }

private:
    struct value_name {
        value_type value;
        std::string_view name;
    };

    std::array<value_name,N> _by_name;
    std::array<value_name,N> _by_value;
    bool _values_are_continues;

    /** Add value and name to table.
     *
     * Used by the constructor.
     */
    template<size_t I, typename... Rest>
    constexpr void add_value_name(value_type value, std::string_view name, Rest const &...rest) noexcept
    {
        std::get<I>(_by_name) = {value, name};
        std::get<I>(_by_value) = {value, name};

        if (sizeof...(Rest)) {
            static_assert(sizeof...(Rest) >= 2);
            add_value_name<I+1>(rest...);
        }
    }

    /** Check if the values are continues
     *
     * Used by the constructor.
     */
    [[nodiscard]] constexpr bool check_values_are_continues() const noexcept
    {
        auto it = _by_value.begin();
        tt_axiom(it != _by_value.end());

        auto check_value = to_underlying(it->value);
        for (; it != _by_value.end(); ++it) {
            if (to_underlying(it->value) != ++check_value) {
                return false;
            }
        }
        return true;
    }
};

template<typename T, typename... Rest>
enum_names(T const &, Rest const &...) -> enum_names<T, sizeof...(Rest) + 1 / 2>;

}

