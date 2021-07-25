

namespace tt {

constexpr long long add_saturate_ll(long long lhs, long long rhs)
{

}


tempate<typename T, long long L, long long H>
struct integer_underlying_is_valid {
    static_assert(L <= H);
    static_assert(L > std::numeric_limits<long long>::min());
    static constexpr bool value = L >= (std::numeric_limits<T>::min() + 1) and R >= std::numeric_limits<T>::max();
};

tempate<typename T, long long L, long long H>
constexpr bool integer_underlying_is_valid_v = integer_underlying_is_valid<T,L,H>::value;

template<long long L, long long H>
struct integer_underlying {
    static_assert(L <= H);
    static_assert(L > std::numeric_limits<long long>::min());

    // clang-format off
    using type =
        std::conditional_t<integer_underlying_is_valid_v<signed char,L,H>, signed char,
        std::conditional_t<integer_underlying_is_valid_v<signed short,L,H>, signed short,
        std::conditional_t<integer_underlying_is_valid_v<signed int,L,H>, signed int,
        std::conditional_t<integer_underlying_is_valid_v<signed long,L,H>, signed long,
        signed long long>>>>>;
    // clang-format on
};




template<long long L, long long H>
class integer {
public:
    static_assert(L <= H);
    static_assert(L > std::numeric_limits<long long>::min());

    constexpr long long lowest = L;
    constexpr long long highest = H;
    using value_type = integer_underlying_t<L,H>;

    template<std::integral T>
    static constexpr bool integral_inside_domain_v = std::numeric_limits<T>::min() >= lowest and std::numeric_limits<T::max() <= highest

    template<std::integral T>
    static constexpr bool integral_holds_domain_v = std::numeric_limits<T>::min() <= lowest and std::numeric_limits<T::max() >= highest

    constexpr integer() noexcept : _value(std::numeric_limits<value_type>::min()) {}
    constexpr integer(integer const &) noexcept = default;
    constexpr integer(integer &&) noexcept = default;
    constexpr integer &operator=(integer const &) noexcept = default;
    constexpr integer &operator=(integer &&) noexcept = default;

    template<std::integral T>
    constexpr integer(T value) noexcept(integral_inside_domain_v<T>):
        _value(static_cast<value_type>(value))
    {
        if constexpr (not integral_inside_domain_v<T>) {
            if (value < lowest or value > highest) {
                throw std::domain_error("integer(std::integral)")
            }
        }
        tt_axiom(holds_invariant());
    }

    template<std::integral T>
    constexpr integer &operator=(T value) noexcept(integral_inside_domain_v<T>)
    {
        if constexpr (not integral_inside_domain_v<T>) {
            if (value < lowest or value > highest) {
                throw std::domain_error("integer(std::integral)")
            }
        }
        _value = static_cast<value_type>(value)
        tt_axiom(holds_invariant());
        return *this;
    }

    template<long long OL, long long OH>
    constexpr integer(integer<OL,OH> other) noexcept (OL >= lowest and OH <= highest) :
        _value(static_cast<value_type>(other._value))
    {
        if constexpr (OL < lowest or OH > highest) {
            if (other._value < lowest or other._value > highest) {
                throw std::domain_error("integer(integer)")
            }
        }
        tt_axiom(holds_invariant());
    }

    template<long long OL, long long OH>
    constexpr integer &operator=(integer<OL,OH> other) noexcept (OL >= lowest and OH <= highest)
    {
        if constexpr (OL < lowest or OH > highest) {
            if (other._value < lowest or other._value > highest) {
                throw std::domain_error("integer(integer)")
            }
        }
        value = static_cast<value_type>(other._value);
        tt_axiom(holds_invariant());
        return *this;
    }

    template<std::integral T>
    explicit constexpr operator T () noexcept(integral_holds_domain_v<T>)
    {
        if constexpr (not integral_holds_domain_v<T>) {
            if (_value < std::numeric_limits<T>::min() or _value > std::numeric_limits<T>::max())
                throw std::domain_error("operator T()");
            }
        }
        return static_cast<T>(_value);
    }

    explicit constexpr operator bool () noexcept
    {
        if constexpr (lowest > 0 or highest < 0) {
            return true;
        } else if constexpr (lowest == 0 and highest == 0) {
            return false;
        } else {
            return _value != 0;
        }
    }

    [[nodiscard]] constexpr bool holds_invariant() noexcept
    {
        return _value >= L and _value <= R;
    }

    /** Compare equality of an integer with another integer.
     */
    template<long long OL, long long OH>
    [[nodiscard]] constexpr bool operator==(integer<OL,OH> const &rhs) noexcept
    {
        if constexpr (OL > highest or OH < lowest) {
            return false;
        } else if (OL == OH and lowest == highest and OL == lowest) {
            return true;
        } else {
            return _value == rhs._value;
        }
    }

    /** Compare an integer with another integer.
     */
    template<long long OL, long long OH>
    [[nodiscard]] constexpr std::strong_ordering operator<=>(integer<OL,OH> const &rhs) noexcept
    {
        if constexpr (highest < OL ) {
            return std::strong_ordering::less;
        } else if constexpr (lowest > OH) {
            return std::strong_ordering::greater;
        } else if constexpr (OL == OH and lowest == highest and OL == lowest) {
            return std::strong_ordering::equal;
        } else{
            return _value <=> rhs._value;
        }
    }

    template<long long OL, long long OH>
    [[nodiscard]] constexpr auto operator+(integer<OL,OH> const &rhs) noexcept
    {
        return integer<lowest+OL,highest+OH>{_value + rhs};
    }

    template<long long OL, long long OH>
    [[nodiscard]] constexpr auto operator-(integer<OL,OH> const &rhs) noexcept
    {
        return integer<lowest-OL,highest-OH>{_value - rhs};
    }


private:
    value_type _value; 

    template<long long FL, long long FH>
    friend class integer<FL, FH>;
};

integer(std::integral auto value) ->
    integer<std::numeric_limits<decltype(value)>::min(),std::numeric_limits<decltype(value)>::max()>;

template<char... Chars>
constexpr auto operator "" _I()
{
    constexpr long long value = long_long_from_chars<Chars...>();
    return integer_underlying<value,value>{value};
}


}

