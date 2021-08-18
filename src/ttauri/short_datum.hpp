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
#include <vector>
#include <unordered_map>

namespace tt {
namespace detail {

/** A 64 bit dynamic data type.
 *
 *
 * By default a datum is encoded as a double-precision floating-point value. Other
 * types are encoded in the mantissa-and-sign when the exponent is all '1'.
 *
 *   S | code  | sub | type
 *  :- |:----- |:--- |:--------
 *   0 | \-000 |     | +infinite or NaN.
 *   0 | \-001 |     | 48 bit signed integral.
 *   0 | \-010 |     | 40+8 bit decimal.
 *   0 | \-011 |     | `std::chrono::year_month_day`
 *   0 | \-100 |     | 6 char string encoded lsb to msb in uint64_t
 *   0 | \-101 | sss | 0-5 char string encoded lsb to msb in uint64_t
 *   0 | \-110 |     |
 *   0 | \-111 | 000 | false
 *   0 | \-111 | 001 | true
 *   0 | \-111 | 010 | null
 *   0 | \-111 | 011 | monostate
 *   0 | \-111 | 100 |
 *   0 | \-111 | 101 |
 *   0 | \-111 | 110 | flow-continue
 *   0 | \-111 | 111 | flow-break
 *   1 |  00   |     | -infinite or signaling NaN.
 *   1 |  01   |     | `std::string *`
 *   1 |  10   |     | `std::vector<datum> *`
 *   1 |  11   |     | `std::unordered_map<datum,datum> *`
 *
 * Pointers are encoded when the sign bit is '1'; the 2 msb bits of the mantissa
 * are used to encoded the pointer type. The pointer itself is stored in the
 * remaining 50 bits.
 *
 * Intel 5-level paging gives us a pointer of 57 bits. Bits [63:56] are
 * zero in user space programs on Linux and Windows. This leaves us with
 * 56 - 50 = 6 bits short. With an alignment to 64 bytes the bottom 6 bits
 * will be zero.
 *
 * ARM64 A pointer is 48 bits, with an extra 8 bits [63:56] used for tagging pointers.
 * As previous, bits [55:47] are '0' for user space programs, for a total of 55 bits.
 * This leaves us with 55 - 50 = 5 bits short. With an alignment to 32 bytes the bottom
 * 5 bits will be zero.
 *
 * ARM64 Also has an extended format which is 52 bits. This will not be supported yet,
 * or maybe through a special option.
 * ARM64 Also supports pointer authentication which uses all unused upper bits. On Linux
 * this is only supported for instruction pointers. These will not be supported by datum,
 * unless we extent datum to 128 bits.
 */
class datum {
public:
    using datum_vector = std::vector<datum>;
    using datum_map = std::unordered_map<datum, datum>;

    constexpr ~datum() noexcept
    {
        delete_pointer();
    }

    constexpr datum(datum const &other) noexcept : _tag(other._tag), _value(other._value)
    {
        if (other.is_pointer()) {
            copy_pointer(other);
        }
    }

    constexpr datum(datum &&other) noexcept : _tag(other._tag), _value(other._value)
    {
        other._tag = tag_type::monostate;
        other._value = 0;
    }

    constexpr datum() noexcept : _tag(tag_type::monostate), _value(0) {}
    constexpr explicit datum(std::monostate) noexcept : _tag(tag_type::monostate), _value(0) {}
    constexpr explicit datum(nullptr_t) noexcept : _tag(tag_type::null), _value(0) {}
    constexpr explicit datum(bool value) noexcept : _tag(tag_type::boolean), _value(value) {}
    constexpr explicit datum(std::floating_point auto value) noexcept :
        _tag(tag_type::floating_point), _value(static_cast<double>(value))
    {
    }

    constexpr explicit datum(numeric_integral auto value) noexcept :
        _tag(tag_type::integral), _value(static_cast<long long>(value))
    {
    }

    constexpr explicit datum(decimal value) noexcept : _tag(tag_type::decimal), _value(value) {}

    constexpr explicit datum(std::chrono::year_month_day value) noexcept : _tag(tag_type::year_month_day), _value(value) {}

    constexpr explicit datum(std::string value) noexcept : _tag(tag_type::string), _value(new std::string{std::move(value)}) {}
    constexpr explicit datum(datum_vector value) noexcept : _tag(tag_type::vector), _value(new datum_vector{std::move(value)}) {}
    constexpr explicit datum(datum_map value) noexcept : _tag(tag_type::map), _value(new datum_map{std::move(value)}) {}
    constexpr explicit datum(URL value) noexcept : _tag(tag_type::url), _value(new URL{std::move(value)}) {}
    constexpr explicit datum(bstring value) noexcept : _tag(tag_type::bstring), _value(new bstring{std::move(value)}) {}

    constexpr datum &operator=(datum const &other) noexcept
    {
        tt_return_on_self_assignment(other);

        delete_pointer();
        _tag = other._tag;
        _value = other._value;
        if (other.is_pointer) {
            copy_pointer(other);
        }
    }

    constexpr datum &operator=(datum &&other) noexcept
    {
        std::swap(_tag, other._tag);
        std::swap(_value, other._value);
    }

    constexpr explicit datum &operator=(std::floating_point auto value) noexcept(sizeof(value) <= 4)
    {
        delete_pointer();
        _tag = tag_type::floating_point;
        _value = static_cast<double>(value);
        return *this;
    }

    constexpr explicit datum &operator=(numeric_integral auto value) noexcept(sizeof(value) <= 4)
    {
        delete_pointer();
        _tag = tag_type::integral;
        _value = static_cast<long long>(value);
        return *this;
    }

    constexpr explicit datum &operator=(decimal value)
    {
        delete_pointer();
        _tag = tag_type::decimal;
        _value = value;
        return *this;
    }
    constexpr explicit datum &operator=(bool value) noexcept
    {
        delete_pointer();
        _tag = tag_type::boolean;
        _value = value;
        return *this;
    }

    constexpr explicit datum &operator=(std::chrono::year_month_day value) noexcept
    {
        delete_pointer();
        _tag = tag_type::year_month_day;
        _value = value;
        return *this;
    }

    constexpr explicit datum &operator=(std::monostate) noexcept
    {
        delete_pointer();
        _tag = tag_type::monostate;
        _value = 0;
        return *this;
    }

    constexpr explicit datum &operator=(nullptr_t) noexcept
    {
        delete_pointer();
        _tag = tag_type::null;
        _value = 0;
        return *this;
    }

    constexpr explicit datum &operator=(std::string value) noexcept
    {
        delete_pointer();
        _tag = tag_type::string;
        _value = new std::string{std::move(value)};
        return *this;
    }

    constexpr explicit datum &operator=(datum_vector value) noexcept
    {
        delete_pointer();
        _tag = tag_type::vector;
        _value = new datum_vector{std::move(value)};
        return *this;
    }

    constexpr explicit datum &operator=(datum_map value) noexcept
    {
        delete_pointer();
        _tag = tag_type::map;
        _value = new datum_vector{std::move(value)};
        return *this;
    }

    constexpr explicit datum &operator=(URL value) noexcept
    {
        delete_pointer();
        _tag = tag_type::url;
        _value = new datum_vector{std::move(value)};
        return *this;
    }

    constexpr explicit datum &operator=(bstring value) noexcept
    {
        delete_pointer();
        _tag = tag_type::bstring;
        _value = new bstring{std::move(value)};
        return *this;
    }

    constexpr explicit operator bool() const noexcept
    {
        switch (_tag) {
        case tag_type::floating_point: return static_cast<bool>(get<double>(*this));
        case tag_type::decimal: return static_cast<bool>(get<decimal>(*this));
        case tag_type::boolean: return get<bool>(*this);
        case tag_type::integral: return static_cast<bool>(get<long long>(*this));
        case tag_type::year_month_day: return true;
        case tag_type::string: return static_cast<bool>(std::size(get<std::string>(*this)));
        case tag_type::vector: return static_cast<bool>(std::size(get<datum_vector>(*this)));
        case tag_type::map: return static_cast<bool>(std::size(get<datum_map>(*this)));
        case tag_type::URL: return static_cast<bool>(get<URL>(*this));
        case tag_type::bstring: return static_cast<bool>(std::size(get<bstring>(*this)));
        default: return false;
        }
    }

    template<std::floating_point T>
    constexpr explicit operator T() const
    {
        switch (_tag) {
        case tag_type::floating_point: return static_cast<T>(get<double>(*this));
        case tag_type::integral: return static_cast<T>(get<long long>(*this));
        case tag_type::decimal: return static_cast<T>(get<decimal>(*this));
        case tag_type::boolean: return static_cast<T>(get<bool>(*this));
        default: throw std::domain_error(std::format("Can't convert {} to floating point", *this));
        }
    }

    template<numeric_integral T>
    constexpr explicit operator T() const
    {
        switch (_tag) {
        case tag_type::floating_point: {
            errno = 0;
            auto r = std::round(get<double>(*this));
            if (errno == EDOM or errno == ERANGE or r < std::numeric_limits<T>::min() or r > std::numeric_limits<T>::max()) {
                throw std::overflow_error("double to integral");
            }
            return static_cast<T>(r);
        }
        case tag_type::integral: {
            auto r = get<long long>(*this);
            if (r < std::numeric_limits<T>::min() or r > std::numeric_limits<T>::max()) {
                throw std::overflow_error("double to integral");
            }
            return static_cast<T>(r);
        }
        case tag_type::decimal: {
            auto r = static_cast<long long>(get<decimal>(*this));
            if (r < std::numeric_limits<T>::min() or r > std::numeric_limits<T>::max()) {
                throw std::overflow_error("double to integral");
            }
            return static_cast<T>(r);
        }
        case tag_type::boolean: return static_cast<T>(get<bool>(*this));
        default: throw std::domain_error(std::format("Can't convert {} to an integral", *this));
        }
    }

    constexpr explicit operator std::chrono::year_month_day() const
    {
        if (_tag != tag_type::year_month_day) {
            throw std::domain_error(std::format("Can't convert {} to an std::chrono::year_month_day", *this));
        }
        return get<std::chrono::year_month_day>(*this);
    }

    constexpr explicit operator std::string() noexcept const
    {
        // XXX should handle every type.
        if (_tag != tag_type::string) {
            throw std::domain_error(std::format("Can't convert {} to an std::string", *this));
        }
        return get<std::string>(*this);
    }

    constexpr explicit operator datum_vector() const
    {
        // XXX should be able to copy the keys of a map sorted.
        if (_tag != tag_type::vector) {
            throw std::domain_error(std::format("Can't convert {} to an datum_vector", *this));
        }
        return get<datum_vector>(*this);
    }

    constexpr explicit operator datum_map() const
    {
        if (_tag != tag_type::map) {
            throw std::domain_error(std::format("Can't convert {} to an datum_map", *this));
        }
        return get<datum_map>(*this);
    }

    constexpr explicit operator URL() const
    {
        // XXX should be able to cast from a std::string.
        if (_tag != tag_type::url) {
            throw std::domain_error(std::format("Can't convert {} to an URL", *this));
        }
        return get<URL>(*this);
    }

    constexpr explicit operator bstring() const
    {
        // XXX should be able to base-64 decode a std::string.
        if (_tag != tag_type::bstring) {
            throw std::domain_error(std::format("Can't convert {} to an bstring", *this));
        }
        return get<bstring>(*this);
    }

#define X(op) \
    constexpr datum &operator op(auto const &rhs) \
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

    [[nodiscard]] friend constexpr bool operator==(datum const &lhs, datum const &rhs) noexcept
    {
        if (auto doubles = promote_if<double>(lhs, rhs)) {
            if (isnan(doubles.lhs) and isnan(doubles.rhs)) {
                // Datums are strongly ordered, so two NaNs must compare equal.
                return true;
            } else {
                return doubles.lhs == doubles.rhs;
            }

        } else if (auto decimals = promote_if<decimal>(lhs, rhs)) {
            return decimals.lhs == decimals.rhs;

        } else if (auto long_longs = promote_if<long long>(lhs, rhs)) {
            return long_longs.lhs == long_longs.rhs;

        } else if (auto bools = promote_if<bool>(lhs, rhs)) {
            return bools.lhs == bools.rhs;

        } else if (auto ymds = promote_if<std::chrono::year_month_day>(lhs, rhs)) {
            return ymds.lhs == ymds.rhs;

        } else if (auto urls = promote_if<URL>(lhs, rhs)) {
            return urls.lhs == urls.rhs;

        } else if (auto strings = promote_if<std::string>(lhs, rhs)) {
            return strings.lhs == strings.rhs;

        } else if (auto vectors = promote_if<datum_vector>(lhs, rhs)) {
            return vectors.lhs == vectors.rhs;

        } else if (auto maps = promote_if < datum_map(lhs, rhs)) {
            return maps.lhs == maps.rhs;

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
     * - monostate
     * - flow continue
     * - flow break
     */
    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(datum const &lhs, datum const &rhs) noexcept
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

        } else if (lhs.type() == detail::datum_type::simple and rhs.type() == detail::datum_type::simple) {
            return subtype() <=> subtype();

        } else {
            return type() <=> type();
        }
    }

#define X(op) \
    [[nodiscard]] constexpr datum operator op(datum const &lhs, datum const &rhs) \
    { \
        if (not(lhs.is_numeric() and rhs.is_numeric())) { \
            throw std::domain_error("Could not " #op " non-numeric arguments"); \
        } \
        if constexpr (#op[0] == '/' or #op[0] == '%') { \
            if (static_cast<double>(rhs) == 0.0) { \
                throw std::domain_error("Divide by zero"); \
            } \
        } \
        datum r; \
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
    [[nodiscard]] constexpr datum operator op(datum const &lhs, datum const &rhs) \
    { \
        datum r; \
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
    [[nodiscard]] constexpr datum operator op(datum const &lhs, datum const &rhs) \
    { \
        datum r; \
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
    [[nodiscard]] friend constexpr auto operator op(datum const &lhs, auto const &rhs) noexcept(noexcept(lhs op datum{rhs})) \
    { \
        return lhs op datum{rhs}; \
    } \
    [[nodiscard]] friend constexpr auto operator op(auto const &lhs, datum const &rhs) noexcept(noexcept(datum{lhs} op rhs)) \
    { \
        return datum{lhs} op rhs; \
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
     * @tparam T Type to check.
     * @param rhs The datum to check the value-type of.
     * @return True if the value-type matches the template parameter @a T
     */
    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative(datum const &rhs) noexcept;

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<double>(datum const &rhs) noexcept
    {
        return _tag == tag_type::floating_point;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<long long>(datum const &rhs) noexcept
    {
        return _tag == tag_type::integral;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<decimal>(datum const &rhs) noexcept
    {
        return _tag == tag_type::decimal;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<bool>(datum const &rhs) noexcept
    {
        return _tag == tag_type::boolean;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<std::chrono::year_month_day>(datum const &rhs) noexcept
    {
        return _tag == tag_type::year_month_day;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<nullptr_t>(datum const &rhs) noexcept
    {
        return _tag == tag_type::null;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<std::monostate>(datum const &rhs) noexcept
    {
        return _tag == tag_type::monostate;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<std::string>(datum const &rhs) noexcept
    {
        return _tag == tag_type::string;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<datum_vector>(datum const &rhs) noexcept
    {
        return _tag == tag_type::vector;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<datum_map>(datum const &rhs) noexcept
    {
        return _tag == tag_type::map;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<url>(datum const &rhs) noexcept
    {
        return _tag == tag_type::url;
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative<bstring>(datum const &rhs) noexcept
    {
        return _tag == tag_type::bstring;
    }

    /** Get the value of a datum.
     *
     * It is monostate behavior if the type does not match the stored value.
     *
     * @tparam T Type to check, must be one of: `bool`, `double`, `long long`, `decimal`, `std::chrono::year_month_day`
     * @param rhs The datum to get the value from.
     * @return A copy of the value in the datum.
     */
    template<typename T>
    [[nodiscard]] friend constexpr T get(datum const &rhs) const noexcept;

    template<typename T>
    [[nodiscard]] friend constexpr T get(datum const &rhs) noexcept;

    template<>
    [[nodiscard]] friend constexpr double const &get<double>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::integral);
        return _double;
    }

    template<>
    [[nodiscard]] friend constexpr double &get<double>(datum const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::integral);
        return _double;
    }

    template<>
    [[nodiscard]] friend constexpr long long const &get<long long>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::integral);
        return _long_long;
    }

    template<>
    [[nodiscard]] friend constexpr long long &get<long long>(datum const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::integral);
        return _long_long;
    }

    template<>
    [[nodiscard]] friend constexpr decimal const &get<decimal>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::decimal);
        return _decimal;
    }

    template<>
    [[nodiscard]] friend constexpr decimal &get<decimal>(datum const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::decimal);
        return _decimal;
    }

    template<>
    [[nodiscard]] friend constexpr bool const &get<bool>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::boolean);
        return _bool;
    }

    template<>
    [[nodiscard]] friend constexpr bool &get<bool>(datum const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::boolean);
        return _bool;
    }

    template<>
    [[nodiscard]] friend constexpr std::chrono::year_month_day const &
    get<std::chrono::year_month_day>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::year_month_day);
        return _year_month_day;
    }

    template<>
    [[nodiscard]] friend constexpr std::chrono::year_month_day &get<std::chrono::year_month_day>(datum const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::year_month_day);
        return _year_month_day;
    }

    template<>
    [[nodiscard]] friend constexpr std::string const &get<std::string>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::string);
        tt_axiom(_string);
        return *_string;
    }

    template<>
    [[nodiscard]] friend constexpr std::string &get<std::string>(datum const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::string);
        tt_axiom(_string);
        return *_string;
    }

    template<>
    [[nodiscard]] friend constexpr datum_vector const &get<datum_vector>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::vector);
        tt_axiom(_vector);
        return *_vector;
    }

    template<>
    [[nodiscard]] friend constexpr datum_vector &get<datum_vector>(datum const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::vector);
        tt_axiom(_vector);
        return *_vector;
    }

    template<>
    [[nodiscard]] friend constexpr datum_map const &get<datum_map>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::map);
        tt_axiom(_map);
        return *_map;
    }

    template<>
    [[nodiscard]] friend constexpr datum_map &get<datum_map>(datum const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::map);
        tt_axiom(_map);
        return *_map;
    }

    template<>
    [[nodiscard]] friend constexpr URL const &get<URL>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::url);
        tt_axiom(_url);
        return *_url;
    }

    template<>
    [[nodiscard]] friend constexpr URL &get<URL>(datum const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::url);
        tt_axiom(_url);
        return *_url;
    }

    template<>
    [[nodiscard]] friend constexpr bstring const &get<bstring>(datum const &rhs) const noexcept
    {
        tt_axiom(_tag == tag_type::bstring);
        tt_axiom(_bstring);
        return *_bstring;
    }

    template<>
    [[nodiscard]] friend constexpr bstring &get<URL>(bstring const &rhs) noexcept
    {
        tt_axiom(_tag == tag_type::bstring);
        tt_axiom(_bstring);
        return *_bstring;
    }

    /** Get the value of a datum.
     *
     * @tparam T Type to check.
     * @param rhs The datum to get the value from.
     * @return A pointer to the value, or nullptr.
     */
    template<typename T>
    [[nodiscard]] friend constexpr T const *get_if(datum const &rhs) const noexcept
    {
        if (holds_alternative<T>(rhs)) {
            return &get<T>(rhs);
        } else {
            return nullptr;
        }
    }

    template<typename T>
    [[nodiscard]] friend constexpr T *get_if(datum const &rhs) noexcept
    {
        if (holds_alternative<T>(rhs)) {
            return &get<T>(rhs);
        } else {
            return nullptr;
        }
    }

private:
    static constexpr int64_t int48_max = 140737488355327LL;
    static constexpr int64_t int48_min = -140737488355328LL;
    static constexpr int64_t int40_max = 549755813887LL;
    static constexpr int64_t int40_min = -549755813888LL;

    enum class tag_type : signed char {
        // scalars are detected by: `to_underlying(tag_type) >= 0`
        monostate = 0,
        floating_point = 1,
        integral = 2,
        decimal = 3,
        boolean = 4,
        null = 5,
        year_month_day = 6,
        flow_continue = 7,
        flow_break = 8,

        // pointers are detected by: `to_underlying(tag_type) < 0`.
        string = -1,
        vector = -2,
        map = -3,
        url = -4,
        bstring = -5,
    };

    template<typename T>
    class promote_value_type {
    public:
        constexpr promote_value_type(promote_result_type const &) noexcept = default;
        constexpr promote_value_type(promote_result_type &&) noexcept = default;
        constexpr promote_value_type &operator=(promote_result_type const &) noexcept = default;
        constexpr promote_value_type &operator=(promote_result_type &&) noexcept = default;

        constexpr promote_result_type() noexcept : _lhs(), _rhs(), _has_result(false) {}

        constexpr operator bool() const noexcept
        {
            return _has_result;
        }

        [[nodiscard]] constexpr T const &lhs() const noexcept
        {
            return _lhs;
        }

        [[nodiscard]] constexpr T const &rhs() const noexcept
        {
            return _rhs;
        }

        constexpr void set(T lhs, T rhs) noexcept
        {
            _lhs = lhs;
            _rhs = rhs;
            _has_result = true;
        }

    private:
        T _lhs;
        T _rhs;

        bool _has_result;
    };

    template<typename T>
    class promote_pointer_type {
    public:
        constexpr ~promote_pointer_type()
        {
            if (_owns_lhs) {
                delete _lhs;
            }
            if (_owns_rhs) {
                delete _rhs;
            }
        }

        promote_pointer_type(promote_pointer_type const &) = delete;
        promote_pointer_type &operator=(promote_pointer_type const &) = delete;

        constexpr promote_pointer_type(promote_pointer_type &&other) noexcept :
            _lhs(other._lhs),
            _rhs(other._rhs),
            _has_result(other._has_result),
            _owns_lhs(std::exchange(other._owns_lhs, false)),
            _owns_rhs(std::exchange(other._owns_rhs, false))
        {
        }

        constexpr promote_pointer_type &operator=(promote_pointer_type &&other)
        {
            _lhs = other._lhs;
            _rhs = other._rhs;
            _has_result = other._has_result;
            _owns_lhs = std::exchange(other._owns_lhs, false);
            _owns_rhs = std::exchange(other._owns_rhs, false);
            return *this;
        }

        constexpr promote_result_type() noexcept :
            _lhs(nullptr), _rhs(nullptr), _has_result(false), _owns_lhs(false), _owns_rhs(false)
        {
        }

        constexpr void set(T const &lhs, T const &rhs) noexcept
        {
            tt_axiom(not _has_result and not _owns_lhs, and not _owns_rhs);
            _lhs = &lhs;
            _rhs = &rhs;
            _has_result = true;
        }

        constexpr void set(T const &lhs, T &&rhs) noexcept
        {
            tt_axiom(not _has_result and not _owns_lhs, and not _owns_rhs);
            _lhs = &lhs;
            _rhs = new T(std::move(rhs));
            _has_result = true;
            _owns_rhs = true;
        }

        constexpr void set(T &&lhs, T const &rhs) noexcept
        {
            tt_axiom(not _has_result and not _owns_lhs, and not _owns_rhs);
            _lhs = new T(std::move(lhs));
            _rhs = &rhs;
            _has_result = true;
            _owns_lhs = true;
        }

        constexpr void set(T &&lhs, T &&rhs) noexcept
        {
            tt_axiom(not _has_result and not _owns_lhs, and not _owns_rhs);
            _lhs = new T(std::move(lhs));
            _rhs = new T(std::move(rhs));
            _has_result = true;
            _owns_lhs = true;
            _owns_rhs = true;
        }

        constexpr operator bool() const noexcept
        {
            return _has_result;
        }

        [[nodiscard]] constexpr T const &lhs() const noexcept
        {
            return *_lhs;
        }

        [[nodiscard]] constexpr T const &rhs() const noexcept
        {
            return *_rhs;
        }

    private:
        T const *_lhs;
        T const *_rhs;

        bool _has_result;
        bool _owns_lhs;
        bool _owns_rhs;
    };

    tag_type _tag = tag_type::monostate;
    union value_type {
        double _double;
        long long _long_long;
        decimal _decimal;
        bool _bool;
        std::chrono::year_month_day _year_month_day;
        std::string *_string;
        datum_vector *_vector;
        datum_map *_map;
    };

    value_type _value;

    [[nodiscard]] constexpr bool is_scalar() const noexcept
    {
        return to_underlying(_tag) >= 0;
    }

    [[nodiscard]] constexpr bool is_pointer() const noexcept
    {
        return to_underlying(_tag) < 0;
    }

    tt_no_inline void copy_pointer(datum const &other) noexcept
    {
        tt_axiom(other.is_pointer());
        switch (other._tag) {
        case tag_type::string: _value._string = new std::string{other._value._string}; return;
        case tag_type::vector: _value._vector = new datum_vector{other._value._vector}; return;
        case tag_type::map: _value._map = new datum_map{other._value._map}; return;
        case tag_type::url: _value._url = new URL{other._value._url}; return;
        case tag_type::bstring: _value._bstring = new bstring{other._value._bstring}; return;
        default: tt_no_default();
        }
    }

    tt_no_inline void _delete_pointer() noexcept
    {
        tt_axiom(is_pointer());
        switch (_tag) {
        case tag_type::string: delete _value._string; return;
        case tag_type::vector: delete _value._vector; return;
        case tag_type::map: delete _value._map; return;
        case tag_type::url: delete _value._url; return;
        case tag_type::bstring: delete _value._bstring; return;
        default: tt_no_default();
        }
    }

    constexpr void delete_pointer() noexcept
    {
        if (is_pointer()) {
            delete_pointer();
        }
    }

    template<typename T>
    [[nodiscard]] friend constexpr bool promotable_to(datum const &rhs) noexcept;

    template<>
    [[nodiscard]] friend constexpr bool promotable_to<double>(datum const &rhs) noexcept
    {
        return holds_alternative<double>(rhs) or holds_alternative<decimal>(rhs) or holds_alternative<long long>(rhs) or
            holds_alternative<bool>(rhs);
    }

    template<>
    [[nodiscard]] friend constexpr bool promotable_to<decimal>(datum const &rhs) noexcept
    {
        return holds_alternative<decimal>(rhs) or holds_alternative<long long>(rhs) or holds_alternative<bool>(rhs);
    }

    template<>
    [[nodiscard]] friend constexpr bool promotable_to<long long>(datum const &rhs) noexcept
    {
        return holds_alternative<long long>(rhs) or holds_alternative<bool>(rhs);
    }

    template<>
    [[nodiscard]] friend constexpr bool promotable_to<bool>(datum const &rhs) noexcept
    {
        return holds_alternative<bool>(rhs);
    }

    template<typename T>
    [[nodiscard]] friend constexpr auto promote_if(datum const &lhs, datum const &rhs) noexcept;

#define X(type, result_type) \
    template<> \
    [[nodiscard]] friend constexpr auto promote_if<double>(datum const &lhs, datum const &rhs) noexcept \
    { \
        auto r = result_type<type>(); \
        if (holds_alternative<type>(lhs) and holds_alternative<type>(rhs)) { \
            r.set(get<type>(lhs), get<type>(rhs)); \
        } else if (holds_alternative<type>(lhs) and promotable_to<type>(rhs)) { \
            r.set(get<type>(lhs), static_cast<type>(rhs)); \
        } else if (promotable_to<type>(lhs) and holds_alternative<type>(rhs)) { \
            r.set(static_cast<type>(lhs), get<type>(rhs)); \
        } \
        return r; \
    }

    X(double, promote_value_type)
    X(decimal, promote_value_type)
    X(long long, promote_value_type)
    X(bool, promote_value_type)
    X(std::chrono::year_month_day, promote_value_type)
    X(std::string, promote_pointer_type)
    X(datum_vector, promote_pointer_type)
    X(datum_map, promote_pointer_type)
    X(URL, promote_pointer_type)
    X(bstring, promote_pointer_type)
#undef X
};

} // namespace tt
