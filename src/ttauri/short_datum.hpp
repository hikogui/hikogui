// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "exception.hpp"
#include "assert.hpp"
#include "concepts.hpp"
#include "required.hpp"
#include "decimal.hpp"
#include <cstdint>
#include <concepts>
#include <bit>
#include <exception>
#include <chrono>
#include <limits>

namespace tt {
namespace detail {

/** The type of the value stored in the short datum.
 *
 * By using only three bits for the type we can handle
 * floating point infinites and quiet and signaling NaNs as long
 * as those NaNs use only the bottom 48 bits for their value.
 */
enum class short_datum_type : uint8_t {
    /** Float must be the first type, so that the bit pattern of
     * infinite and NaN will match.
     */
    floating_point = 0,
    decimal = 1,
    integer = 2,
    year_month_day = 3,

    /** A simple value encodes the value+subtype as the 48 bit value.
     *
     * @see short_datum_subtype
     */
    simple = 4,
};

enum class short_datum_subtype : uint8_t {
    bool_false = 0,
    bool_true = 1,
    null = 2,
    undefined = 3,
    flow_continue = 4,
    flow_break = 5
}

} // namespace detail

class long_datum;

/** A 64 bit dynamic data type.
 *
 * The datum can contain one of the following types:
 * - integer
 * - double
 * - decimal
 * - std::date
 * - boolean
 * - null
 * - undefined
 * - break/continue
 *
 */
class short_datum {
public:
    constexpr short_datum(short_datum const &) noexcept = default;
    constexpr short_datum(short_datum &&) noexcept = default;
    constexpr short_datum() noexcept : _v(make()) {}
    constexpr short_datum(nullptr_t) noexcept : _v(make(nullptr)) {}
    constexpr short_datum(bool value) noexcept : _v(make(value)) {}
    constexpr short_datum(numeric_integral auto value) noexcept(sizeof(value) <= 4) : _v(make(value)) {}
    constexpr short_datum(decimal value) : _v(make(value)) {}

    constexpr short_datum(std::chrono::year_month_day value) noexcept : _v(make(value)) {}

    constexpr short_datum &operator=(short_datum const &) noexcept = default;
    constexpr short_datum &operator=(short_datum &&) noexcept = default;

    constexpr explicit short_datum &operator=(nullptr_t) noexcept
    {
        _v = make(nullptr);
        return *this;
    }

    constexpr explicit short_datum &operator=(bool value) noexcept
    {
        _v = make(value);
        return *this;
    }

    constexpr explicit short_datum &operator=(numeric_integral auto value) noexcept(sizeof(value) <= 4)
    {
        _v = make(value);
        return *this;
    }

    constexpr explicit short_datum &operator=(decimal value)
    {
        _v = make(value);
        return *this;
    }

    constexpr explicit short_datum &operator=(std::chrono::year_month_day value) noexcept
    {
        _v = make(value);
        return *this;
    }

    constexpr explicit operator bool() const noexcept
    {
        switch (type()) {
        case detail::short_datum_type::floating_point: return static_cast<bool>(get<double>(*this));
        case detail::short_datum_type::integer: return static_cast<bool>(get<long long>(*this));
        case detail::short_datum_type::decimal: return static_cast<bool>(get<decimal>(*this));
        case detail::short_datum_type::year_month_day: return true;
        case detail::short_datum_type::simple: return subtype() == detail::short_datum_subtype::bool_true;
        default: tt_no_default();
        }
    }

    template<std::floating_point T>
    constexpr explicit operator T() const
    {
        switch (type()) {
        case detail::short_datum_type::floating_point: return static_cast<T>(get<double>(*this));
        case detail::short_datum_type::integer: return static_cast<T>(get<long long>(*this));
        case detail::short_datum_type::decimal: return static_cast<T>(get<decimal>(*this));
        case detail::short_datum_type::year_month_day: throw std::domain_error("Can't convert year_month_day to an integer");
        case detail::short_datum_type::simple:
            switch (subtype()) {
            case detail::short_datum_subtype::true: return T{1};
            case detail::short_datum_subtype::false: return T{0};
            case detail::short_datum_subtype::null: throw std::domain_error("Can't convert null to integer");
            case detail::short_datum_subtype::undefined: throw std::domain_error("Can't convert undefined to integer");
            case detail::short_datum_subtype::flow_break: throw std::domain_error("Can't convert flow-break to integer");
            case detail::short_datum_subtype::flow_continue: throw std::domain_error("Can't convert flow-continue to integer");
            default: tt_no_default();
            }
        default: tt_no_default();
        }
    }

    template<numeric_integral T>
    constexpr explicit operator T() const
    {
        switch (type()) {
        case detail::short_datum_type::floating_point: {
            errno = 0;
            auto r = std::round(get<double>(*this));
            if (errno == EDOM or errno == ERANGE or r < std::numeric_limits<T>::min() or r > std::numeric_limits<T>::max()) {
                throw std::overflow_error("double to integer");
            }
            return static_cast<T>(r);
        }
        case detail::short_datum_type::integer: {
            auto r = get<long long>(*this);
            if (r < std::numeric_limits<T>::min() or r > std::numeric_limits<T>::max()) {
                throw std::overflow_error("double to integer");
            }
            return static_cast<T>(r);
        }
        case detail::short_datum_type::decimal: {
            auto r = static_cast<long long>(get<decimal>(*this));
            if (r < std::numeric_limits<T>::min() or r > std::numeric_limits<T>::max()) {
                throw std::overflow_error("double to integer");
            }
            return static_cast<T>(r);
        }
        case detail::short_datum_type::year_month_day: throw std::domain_error("Can't convert year_month_day to an integer");
        case detail::short_datum_type::simple:
            switch (subtype()) {
            case detail::short_datum_subtype::true: return T{1};
            case detail::short_datum_subtype::false: return T{0};
            case detail::short_datum_subtype::null: throw std::domain_error("Can't convert null to integer");
            case detail::short_datum_subtype::undefined: throw std::domain_error("Can't convert undefined to integer");
            case detail::short_datum_subtype::flow_break: throw std::domain_error("Can't convert flow-break to integer");
            case detail::short_datum_subtype::flow_continue: throw std::domain_error("Can't convert flow-continue to integer");
            default: tt_no_default();
            }
        default: tt_no_default();
        }
    }

    constexpr explicit operator std::chrono::year_month_day() const
    {
        if (type() != detail::short_datum_type::year_month_day) {
            throw std::domain_error("Can't convert to year-month-day");
        }

        return get<std::chrono::year_month_day>(*this);
    }

#define X(op) \
    constexpr short_datum &operator op(auto const &rhs) \
    { \
        return *this = *this op rhs; \
    }

    X(+=)
    X(-=)
    X(*=)
    X(/=)
    X(%=)
    X(&=)
    X(|=)
    X(^=)
    X(<<=)
    X(>>=)
#undef X

    [[nodiscard]] friend constexpr bool operator==(short_datum const &lhs, short_datum const &rhs) noexcept
    {
        if (lhs.type() == rhs.type()) {
            if (holds_alternative<double>(lhs)) {
                return lhs._v.f64 == rhs._v.f64;
            } else {
                return lhs._v.u64 == rhs._v.u64;
            }
        } else if (holds_alternative<double>(lhs) or holds_alternative<double>(rhs)) {
            ttlet lhs_ = static_cast<double>(lhs);
            ttlet rhs_ = static_cast<double>(rhs);
            if ((lhs_ != lhs_) and (rhs_ != rhs_)) {
                // Datums are strongly ordered, so two NaNs must compare equal.
                return true;
            } else {
                return lhs_ == rhs_;
            }

        } else if (holds_alternative<decimal>(lhs) or holds_alternative<decimal>(rhs)) {
            ttlet lhs_ = static_cast<decimal>(lhs);
            ttlet rhs_ = static_cast<decimal>(rhs);
            return lhs_ == rhs_;

        } else if (holds_alternative<long long>(lhs) or holds_alternative<long long>(rhs)) {
            ttlet lhs_ = static_cast<long long>(lhs);
            ttlet rhs_ = static_cast<long long>(rhs);
            return lhs_ == rhs_;

        } else if (holds_alternative<std::chrono::year_month_day>(lhs) and holds_alternative<std::chrono::year_month_day>(rhs)) {
            return get<std::chrono::year_month_day>(lhs) == get<std::chrono::year_month_day>(rhs);

        } else if (holds_alternative<bool>(lhs) and holds_alternative<bool>(rhs)) {
            return get<bool>(lhs) == get<bool>(rhs);

        } else {
            return false;
        }
    }

    /** Compare datums.
     *
     * First promote numeric datums to the highest of @a lhs and @a rhs, then compare.
     * - promotion order: long long -> decimal -> double.
     * - NaNs compare equal
     * - NaN is lower than any other value or type.
     *
     * If types compare equal, then compare the values of those types.
     *
     * If types are not equal then ordering is as follows:
     * - NaN
     * - numeric
     * - year-month-day
     * - boolean
     * - null
     * - undefined
     * - flow continue
     * - flow break
     */
    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(short_datum const &lhs, short_datum const &rhs) noexcept
    {
        if (holds_alternative<double>(lhs) or holds_alternative<double>(rhs)) {
            ttlet lhs_ = static_cast<double>(lhs);
            ttlet rhs_ = static_cast<double>(rhs);
            if (lhs_ == rhs_ or ((lhs_ != lhs_) and (rhs_ != rhs_))) {
                // Datums are strongly ordered, so two NaNs must compare equal.
                return std::strong_order::equal;

            } else if (lhs_ != lhs_) {
                // NaNs are always the lowest value.
                return std::strong_order::less;

            } else if (rhs_ != rhs_) {
                // NaNs are always the lowest value.
                return std::strong_order::greater;

            } else if (lhs_ < rhs_) {
                return std::strong_order::less;

            } else {
                return std::strong_order::greater;
            }

        } else if (holds_alternative<decimal>(lhs) or holds_alternative<decimal>(rhs)) {
            ttlet lhs_ = static_cast<decimal>(lhs);
            ttlet rhs_ = static_cast<decimal>(rhs);
            return lhs_ <=> rhs_;

        } else if (holds_alternative<long long>(lhs) or holds_alternative<long long>(rhs)) {
            ttlet lhs_ = static_cast<long long>(lhs);
            ttlet rhs_ = static_cast<long long>(rhs);
            return lhs_ <=> rhs_;

        } else if (holds_alternative<std::chrono::year_month_day>(lhs) and holds_alternative<std::chrono::year_month_day>(rhs)) {
            return get<std::chrono::year_month_day>(lhs) <=> get<std::chrono::year_month_day>(rhs);

        } else if (holds_alternative<bool>(lhs) and holds_alternative<bool>(rhs)) {
            return get<bool>(lhs) <=> get<bool>(rhs);

        } else if (lhs.type() == detail::short_datum_type::simple and rhs.type() == detail::short_datum_type::simple) {
            return subtype() <=> subtype();

        } else {
            return type() <=> type();
        }
    }

#define X(op) \
    [[nodiscard]] constexpr short_datum operator op(short_datum const &lhs, short_datum const &rhs) \
    { \
        if (not(lhs.is_numeric() and rhs.is_numeric())) { \
            throw std::domain_error("Could not " #op " non-numeric arguments"); \
        } \
        if constexpr (#op[0] == '/' or #op[0] == '%') { \
            if (static_cast<double>(rhs) == 0.0) { \
                throw std::domain_error("Divide by zero"); \
            } \
        } \
        short_datum r; \
        if (holds_alternative<double>(lhs) or holds_alternative<double>(rhs)) { \
            ttlet lhs_ = static_cast<double>(lhs); \
            ttlet rhs_ = static_cast<double>(rhs); \
            r._v = make(lhs_ op rhs_); \
        } else if (holds_alternative<decimal>(lhs) or holds_alternative<decimal>(rhs)) { \
            ttlet lhs_ = static_cast<decimal>(lhs); \
            ttlet rhs_ = static_cast<decimal>(rhs); \
            r._v = make(lhs_ op rhs_); \
        } else { \
            ttlet lhs_ = static_cast<long long>(lhs); \
            ttlet rhs_ = static_cast<long long>(rhs); \
            r._v = make(lhs_ op rhs_); \
        } \
    }

    X(+)
    X(-)
    X(*)
    X(/)
    X(%)
#undef X

#define X(op) \
    [[nodiscard]] constexpr short_datum operator op(short_datum const &lhs, short_datum const &rhs) \
    { \
        short_datum r; \
        if (holds_alternative<long long>(lhs) and holds_alternative<long long>(rhs)) { \
            ttlet lhs_ = get<long long>(lhs); \
            ttlet rhs_ = get<long long>(rhs); \
            r._v = make(lhs_ op rhs_); \
        } else if (holds_alternative<bool>(lhs) and holds_alternative<bool>(rhs)) { \
            ttlet lhs_ = get<bool>(lhs); \
            ttlet rhs_ = get<bool>(rhs); \
            r._v = make(lhs_ op rhs_); \
        } else { \
            throw std::domain_error("Could not " #op " non-integral or non-boolean arguments"); \
        } \
    }

    X(&)
    X(|)
    X(^)
#undef X

#define X(op) \
    [[nodiscard]] constexpr short_datum operator op(short_datum const &lhs, short_datum const &rhs) \
    { \
        short_datum r; \
        if (holds_alternative<long long>(lhs) and holds_alternative<long long>(rhs)) { \
            ttlet lhs_ = get<long long>(lhs); \
            ttlet rhs_ = get<long long>(rhs); \
            r._v = make(lhs_ op rhs_); \
        } else { \
            throw std::domain_error("Could not " #op " non-integral arguments"); \
        } \
    }

    X(<<)
    X(>>)
#undef X

#define X(op) \
    [[nodiscard]] friend constexpr auto operator op(short_datum const &lhs, auto const &rhs) noexcept( \
        noexcept(lhs op short_datum{rhs})) \
    { \
        return lhs op short_datum{rhs}; \
    } \
    [[nodiscard]] friend constexpr auto operator op(auto const &lhs, short_datum const &rhs) noexcept( \
        noexcept(short_datum{lhs} op rhs)) \
    { \
        return short_datum{lhs} op rhs; \
    }

    X(==)
    X(<=>)
    X(+)
    X(-)
    X(*)
    X(/)
    X(%)
    X(&)
    X(|)
    X(^)
    X(<<)
    X(>>)
#undef X

    /** Check if the datum holds a type.
     *
     * @tparam T Type to check, must be one of: `bool`, `double`, `long long`, `decimal`, `std::chrono::year_month_day`,
     * `nullptr_t`
     * @param rhs The datum to check the value-type of.
     * @return True if the value-type matches the template parameter @a T
     */
    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative(short_datum const &rhs) noexcept;

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<bool>(short_datum const &rhs) noexcept
    {
        return rhs.type() == detail::short_datum_type::simple and
            (rhs.subtype() == detail::short_datum_type::bool_true or rhs.subtype() == detail::short_datum_type::bool_false);
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<nullptr_t>(short_datum const &rhs) noexcept
    {
        return rhs.type() == detail::short_datum_type::simple and rhs.subtype() == detail::short_datum_type::null;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<double>(short_datum const &rhs) noexcept
    {
        return rhs.type() == detail::short_datum_type::floating_point;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<long long>(short_datum const &rhs) noexcept
    {
        return rhs.type() == detail::short_datum_type::integer;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<decimal>(short_datum const &rhs) noexcept
    {
        return rhs.type() == detail::short_datum_type::decimal;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<std::chrono::year_month_day>(short_datum const &rhs) noexcept
    {
        return rhs.type() == detail::short_datum_type::year_month_day;
    }

    /** Get the value of a datum.
     *
     * It is undefined behavior if the type does not match the stored value.
     *
     * @tparam T Type to check, must be one of: `bool`, `double`, `long long`, `decimal`, `std::chrono::year_month_day`
     * @param rhs The datum to get the value from.
     * @return A copy of the value in the datum.
     */
    template<typename T>
    [[nodiscard]] friend constexpr T get(short_datum const &rhs) noexcept;

    template<>
    [[nodiscard]] friend constexpr bool get(short_datum const &rhs) noexcept
    {
        tt_axiom(rhs.type() == detail::short_datum_type::simple);
        tt_axiom(
            rhs.subtype() == detail::short_datum_subtype::bool_true or rhs.subtype() == detail::short_datum_subtype::bool_false);
        return (static_cast<bool>(rhs._v.u64 & 1);
    }

    template<>
    [[nodiscard]] friend constexpr double get(short_datum const &rhs) noexcept
    {
        tt_axiom(rhs.type() == detail::short_datum_type::floating_point);
        return rhs._v.f64;
    }

    template<>
    [[nodiscard]] friend constexpr long long get(short_datum const &rhs) noexcept
    {
        tt_axiom(rhs.type() == detail::short_datum_type::integer);
        return static_cast<long long>(static_cast<int64_t>(rhs._v.u64 << 16) >> 16);
    }

    template<>
    [[nodiscard]] friend constexpr std::chrono::year_month_day get(short_datum const &rhs) noexcept
    {
        tt_axiom(rhs.type() == detail::short_datum_type::year_month_day);
        auto value = rhs._v.u64;
        ttlet day = std::chrono::day{static_cast<unsigned>(static_cast<uint8_t>(value))};
        value >>= 8;
        ttlet month = std::chrono::month{static_cast<unsigned>(static_cast<uint8_t>(value))};
        value >>= 8;
        ttlet year = std::chrono::year{static_cast<int>(static_cast<int32_t>(value))};
        return {year, month, day};
    }

    template<>
    [[nodiscard]] friend constexpr decimal get(short_datum const &rhs) noexcept
    {
        tt_axiom(rhs.type() == detail::short_datum_type::decimal);
        auto value = rhs._v.u64;
        auto exponent = static_cast<int>(static_cast<int8_t>(value));
        value <<= 16;
        auto mantissa = static_cast<long long>(static_cast<int64_t>(value >> (16 + 8)));
        return {exponent, mantissa};
    }

    /** Get the value of a datum.
     *
     * @tparam T Type to check, must be one of: `bool`, `double`, `long long`, `decimal`, `std::chrono::year_month_day`
     * @param rhs The datum to get the value from.
     * @return A std::optional of the value in the datum, or empty if type does not match.
     */
    template<typename T>
    [[nodiscard]] friend constexpr std::optional<T> get_if(short_datum const &rhs) noexcept
    {
        if (holds_alternative<T>(rhs)) {
            return get<T>(rhs);
        } else {
            return {};
        }
    }

private:
    static constexpr int64_t int48_max = 140737488355327LL;
    static constexpr int64_t int48_min = -140737488355328LL;
    static constexpr int64_t int40_max = 549755813887LL;
    static constexpr int64_t int40_min = -549755813888LL;

    union value_type {
        static_assert(sizeof(double) == sizeof(uint64_t));

        double f64;
        uint64_t u64;
    };

    value_type _v;

    [[nodiscard]] constexpr detail::short_datum_type type() const noexcept
    {
        static_assert(sizeof(_v) == sizeof(uint64_t));

        auto type = static_cast<uint16_t>(std::bit_cast<uint64_t>(_v) >> 48);
        if (type & 0x7ff0 == 0x7ff0) {
            // Infinite or NaN. Type is encoded in the lower 3 bits.
            // An infinite or a signaling/quite-NaN (with only bottom 48 bits used) are
            // treated as floating point.
            return static_cast<detail::short_datum_type>(type & 0x7);
        } else {
            return detail::short_datum_type::floating_point;
        }
    }

    [[nodiscard]] constexpr detail::short_datum_subtype subtype() const noexcept
    {
        tt_axiom(type() == detail::short_datum_type::simple);
        return static_cast<detail::short_datum_subtype>(static_cast<uint8_t>(_v.u64));
    }

    [[nodiscard]] static constexpr value_type make(std::floating_point auto value) noexcept
    {
        ttlet value_ = static_cast<double>(value);
        if (value_ != value_) {
            // value_ is NaN, clear the type bits, so that it remains NaN.
            ttlet u64 = std::bit_cast<uint64_t>(value_);
            return std::bit_cast<double>(u64 & 0xfff0'ffff'ffff'ffff);
        }

        return value_;
    }

    [[nodiscard]] static constexpr value_type make(short_datum_type type, uint64_t value = 0) noexcept
    {
        tt_axiom(type != short_datum_type::floating_point);
        tt_axiom(value <= 0xffff'ffff'ffff);

        return ((static_cast<uint64_t>(type) | 0x7ff) << 64) | value;
    }

    [[nodiscard]] static constexpr value_type make(short_datum_subtype value) noexcept
    {
        return make(short_datum_type::simple, static_cast<uint64_t>(value));
    }

    [[nodiscard]] static constexpr value_type make(bool value) noexcept
    {
        return make(short_datum_type::simple, static_cast<short_datum_subtype>(value));
    }

    [[nodiscard]] static constexpr value_type make(nullptr_t value) noexcept
    {
        return make(short_datum_type::simple, short_datum_subtype::null);
    }

    [[nodiscard]] static constexpr value_type make() noexcept
    {
        return make(short_datum_type::simple, short_datum_subtype::undefined);
    }

    [[nodisscard]] static constexpr value_type make(is_numeric_unsigned_integral auto value) noexcept(sizeof(value) <= 4)
    {
        if (value > int48_max) {
            throw std::overflow_error("unsigned int overflow");
        }

        return make(short_datum_type::integer, static_cast<uint64_t>(value));
    }

    [[nodisscard]] static constexpr value_type make(is_numeric_signed_integral auto value) noexcept(sizeof(value) <= 4)
    {
        if (value < int48_min or value > int48_max) {
            throw std::overflow_error("signed int overflow");
        }

        // Sign extent to the largest type, then make it unsigned
        auto value_ = static_cast<unsigned long long>(static_cast<signed long long>(value));
        // Truncate the sign bits so that 48 bits are left over.
        constexpr auto bits_to_strip = sizeof(value_) * CHAR_BIT - 48;
        value_ <<= bits_to_strip;
        value_ >>= bits_to_strip;
        return make(short_datum_type::integer, value_);
    }

    [[nodisscard]] static constexpr value_type make(std::chrono::year_month_day value) noexcept
    {
        ttlet year = static_cast<int32_t>(value.year());
        ttlet month = static_cast<unsigned>(value.month());
        ttlet day = static_cast<unsigned>(value.day());
        tt_axiom(year >= -32768 and year <= 32767);
        tt_axiom(month <= 255);
        tt_axiom(day <= 255);

        uint64_t value_ = static_cast<uint32_t>(year);
        value_ <<= 8;
        value_ |= month;
        value_ <<= 8;
        value_ |= day;
        return make(short_datum_type::year_month_day, value_);
    }

    [[nodisscard]] static constexpr value_type make(decimal value)
    {
        long long m = value.mantissa();
        if (m < int40_min or m > int40_max) {
            throw std::overflow_error("decimal mantissa to large");
        }

        int e = rhs.exponent();

    tt_axiom(e >= std::numeric_limits<int8_t>::min() and e <= std::numeric_limits<int8_t>::max()));
    return make(static_cast<uint8_t>(e) | ((static_cast<uint64_t>(m) << 24) >> 16);
    }

    friend class long_datum;
};

} // namespace tt
