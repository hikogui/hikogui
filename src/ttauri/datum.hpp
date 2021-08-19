// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "exception.hpp"
#include "assert.hpp"
#include "concepts.hpp"
#include "required.hpp"
#include "decimal.hpp"
#include "URL.hpp"
#include "byte_string.hpp"
#include "hash.hpp"
#include <cstdint>
#include <concepts>
#include <bit>
#include <exception>
#include <chrono>
#include <limits>
#include <vector>
#include <unordered_map>

namespace tt {
class datum;
}

namespace std {

template<>
struct hash<tt::datum> {
    [[nodiscard]] constexpr size_t operator()(tt::datum const &rhs) const noexcept;
};

} // namespace std

namespace tt {
namespace detail {

/** Promotion result.
 *
 * @tparam To The type to promote the values to.
 */
template<typename To>
class datum_promotion_result {
public:
    using value_type = To;
    static constexpr bool data_is_pointer = sizeof(value_type) > sizeof(void *);
    static constexpr bool data_is_scalar = not data_is_pointer;

    constexpr void clear() noexcept requires(data_is_scalar) {}
    constexpr void clear() noexcept requires(data_is_pointer)
    {
        if (_owns_lhs) {
            delete _lhs;
        }
        if (_owns_rhs) {
            delete _rhs;
        }
    }

    constexpr ~datum_promotion_result()
    {
        clear();
    }

    constexpr datum_promotion_result() noexcept = default;

    datum_promotion_result(datum_promotion_result const &) = delete;
    datum_promotion_result &operator=(datum_promotion_result const &) = delete;

    constexpr datum_promotion_result(datum_promotion_result &&other) noexcept :
        _lhs(other._lhs),
        _rhs(other._rhs),
        _is_result(other._is_result),
        _owns_lhs(std::exchange(other._owns_lhs, false)),
        _owns_rhs(std::exchange(other._owns_rhs, false))
    {
    }

    constexpr datum_promotion_result &operator=(datum_promotion_result &&other) noexcept
    {
        clear();
        _lhs = other.lhs;
        _rhs = other.rhs;
        _is_result = other._is_result;
        _owns_lhs = std::exchange(other._owns_lhs, false);
        _owns_rhs = std::exchange(other._owns_rhs, false);
        return *this;
    }

    constexpr explicit operator bool() const noexcept
    {
        return _is_result;
    }

    constexpr void set(value_type lhs, value_type rhs) noexcept requires(data_is_scalar)
    {
        _lhs = lhs;
        _rhs = rhs;
        _is_result = true;
    }

    constexpr void set(value_type const &lhs, value_type const &rhs) noexcept requires(data_is_pointer)
    {
        _lhs = &lhs;
        _rhs = &rhs;
        _is_result = true;
    }

    constexpr void set(value_type &&lhs, value_type const &rhs) noexcept requires(data_is_pointer)
    {
        _lhs = new value_type(std::move(lhs));
        _rhs = &rhs;
        _is_result = true;
        _owns_lhs = true;
    }

    constexpr void set(value_type const &lhs, value_type &&rhs) noexcept requires(data_is_pointer)
    {
        _lhs = &lhs;
        _rhs = new value_type(std::move(rhs));
        _is_result = true;
        _owns_rhs = true;
    }

    constexpr void set(value_type &&lhs, value_type &&rhs) noexcept requires(data_is_pointer)
    {
        _lhs = new value_type(std::move(lhs));
        _rhs = new value_type(std::move(rhs));
        _is_result = true;
        _owns_lhs = true;
        _owns_rhs = true;
    }

    [[nodiscard]] constexpr value_type const &lhs() const noexcept requires(data_is_pointer)
    {
        tt_axiom(_is_result);
        return *_lhs;
    }

    [[nodiscard]] constexpr value_type const &rhs() const noexcept requires(data_is_pointer)
    {
        tt_axiom(_is_result);
        return *_rhs;
    }

    [[nodiscard]] constexpr value_type lhs() const noexcept requires(data_is_scalar)
    {
        tt_axiom(_is_result);
        return _lhs;
    }

    [[nodiscard]] constexpr value_type rhs() const noexcept requires(data_is_scalar)
    {
        tt_axiom(_is_result);
        return _rhs;
    }

private :

    using data_type = std::conditional_t<data_is_pointer, value_type const *, value_type>;
    data_type _lhs = data_type{};
    data_type _rhs = data_type{};
    bool _is_result = false;
    bool _owns_lhs = false;
    bool _owns_rhs = false;
};

} // namespace detail

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
    using vector_type = std::vector<datum>;
    using map_type = std::unordered_map<datum, datum>;
    struct break_type {};
    struct continue_type {};

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
        other._value._long_long = 0;
    }

    constexpr datum() noexcept : _tag(tag_type::monostate), _value(0) {}
    constexpr explicit datum(std::monostate) noexcept : _tag(tag_type::monostate), _value(0) {}
    constexpr explicit datum(nullptr_t) noexcept : _tag(tag_type::null), _value(0) {}
    constexpr explicit datum(continue_type) noexcept : _tag(tag_type::flow_continue), _value(0) {}
    constexpr explicit datum(break_type) noexcept : _tag(tag_type::flow_break), _value(0) {}
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
    explicit datum(std::string value) noexcept : _tag(tag_type::string), _value(new std::string{std::move(value)}) {}
    explicit datum(std::string_view value) noexcept : _tag(tag_type::string), _value(new std::string{value}) {}
    explicit datum(char const *value) noexcept : _tag(tag_type::string), _value(new std::string{value}) {}
    explicit datum(vector_type value) noexcept : _tag(tag_type::vector), _value(new vector_type{std::move(value)}) {}
    explicit datum(map_type value) noexcept : _tag(tag_type::map), _value(new map_type{std::move(value)}) {}
    explicit datum(URL value) noexcept : _tag(tag_type::url), _value(new URL{std::move(value)}) {}
    explicit datum(bstring value) noexcept : _tag(tag_type::bstring), _value(new bstring{std::move(value)}) {}

    template<typename... Args>
    [[nodiscard]] static datum make_vector(Args const &...args) noexcept
    {
        return datum{vector_type{datum{args}...}};
    }

    [[nodiscard]] static datum make_map() noexcept
    {
        return datum{map_type{}};
    }

    [[nodiscard]] static datum make_break() noexcept
    {
        return datum{break_type{}};
    }

    [[nodiscard]] static datum make_continue() noexcept
    {
        return datum{continue_type{}};
    }

    constexpr datum &operator=(datum const &other) noexcept
    {
        tt_return_on_self_assignment(other);

        delete_pointer();
        _tag = other._tag;
        _value = other._value;
        if (other.is_pointer()) {
            copy_pointer(other);
        }
        return *this;
    }

    constexpr datum &operator=(datum &&other) noexcept
    {
        std::swap(_tag, other._tag);
        std::swap(_value, other._value);
        return *this;
    }

    constexpr datum &operator=(std::floating_point auto value) noexcept(sizeof(value) <= 4)
    {
        delete_pointer();
        _tag = tag_type::floating_point;
        _value = static_cast<double>(value);
        return *this;
    }

    constexpr datum &operator=(numeric_integral auto value) noexcept(sizeof(value) <= 4)
    {
        delete_pointer();
        _tag = tag_type::integral;
        _value = static_cast<long long>(value);
        return *this;
    }

    constexpr datum &operator=(decimal value)
    {
        delete_pointer();
        _tag = tag_type::decimal;
        _value = value;
        return *this;
    }
    constexpr datum &operator=(bool value) noexcept
    {
        delete_pointer();
        _tag = tag_type::boolean;
        _value = value;
        return *this;
    }

    constexpr datum &operator=(std::chrono::year_month_day value) noexcept
    {
        delete_pointer();
        _tag = tag_type::year_month_day;
        _value = value;
        return *this;
    }

    constexpr datum &operator=(std::monostate) noexcept
    {
        delete_pointer();
        _tag = tag_type::monostate;
        _value = 0;
        return *this;
    }

    constexpr datum &operator=(nullptr_t) noexcept
    {
        delete_pointer();
        _tag = tag_type::null;
        _value = 0;
        return *this;
    }

    datum &operator=(std::string value) noexcept
    {
        delete_pointer();
        _tag = tag_type::string;
        _value = new std::string{std::move(value)};
        return *this;
    }

    datum &operator=(char const *value) noexcept
    {
        delete_pointer();
        _tag = tag_type::string;
        _value = new std::string{value};
        return *this;
    }

    datum &operator=(std::string_view value) noexcept
    {
        delete_pointer();
        _tag = tag_type::string;
        _value = new std::string{value};
        return *this;
    }

    datum &operator=(vector_type value) noexcept
    {
        delete_pointer();
        _tag = tag_type::vector;
        _value = new vector_type{std::move(value)};
        return *this;
    }

    datum &operator=(map_type value) noexcept
    {
        delete_pointer();
        _tag = tag_type::map;
        _value = new map_type{std::move(value)};
        return *this;
    }

    datum &operator=(URL value) noexcept
    {
        delete_pointer();
        _tag = tag_type::url;
        _value = new URL{std::move(value)};
        return *this;
    }

    datum &operator=(bstring value) noexcept
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
        case tag_type::vector: return static_cast<bool>(std::size(get<vector_type>(*this)));
        case tag_type::map: return static_cast<bool>(std::size(get<map_type>(*this)));
        case tag_type::url: return static_cast<bool>(get<URL>(*this));
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

    constexpr explicit operator decimal() const
    {
        switch (_tag) {
        case tag_type::floating_point: return decimal(get<double>(*this));
        case tag_type::integral: return decimal(get<long long>(*this));
        case tag_type::decimal: return get<decimal>(*this);
        case tag_type::boolean: return decimal(get<bool>(*this));
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
                throw std::overflow_error("long long to integral");
            }
            return static_cast<T>(r);
        }
        case tag_type::decimal: {
            auto r = static_cast<long long>(get<decimal>(*this));
            if (r < std::numeric_limits<T>::min() or r > std::numeric_limits<T>::max()) {
                throw std::overflow_error("decimal to integral");
            }
            return static_cast<T>(r);
        }
        case tag_type::boolean: return static_cast<T>(get<bool>(*this));
        default: throw std::domain_error(std::format("Can't convert {} to an integral", repr(*this)));
        }
    }

    constexpr explicit operator std::chrono::year_month_day() const
    {
        if (_tag != tag_type::year_month_day) {
            throw std::domain_error(std::format("Can't convert {} to an std::chrono::year_month_day", repr(*this)));
        }
        return get<std::chrono::year_month_day>(*this);
    }

    explicit operator std::string() const noexcept
    {
        // XXX should handle every type.
        if (_tag != tag_type::string) {
            tt_not_implemented();
        }
        return get<std::string>(*this);
    }

    explicit operator vector_type() const
    {
        // XXX should be able to copy the keys of a map sorted.
        if (_tag != tag_type::vector) {
            throw std::domain_error(std::format("Can't convert {} to an vector", repr(*this)));
        }
        return get<vector_type>(*this);
    }

    explicit operator map_type() const
    {
        if (_tag != tag_type::map) {
            throw std::domain_error(std::format("Can't convert {} to an map", repr(*this)));
        }
        return get<map_type>(*this);
    }

    explicit operator URL() const
    {
        // XXX should be able to cast from a std::string.
        if (_tag != tag_type::url) {
            throw std::domain_error(std::format("Can't convert {} to an URL", repr(*this)));
        }
        return get<URL>(*this);
    }

    explicit operator bstring() const
    {
        // XXX should be able to base-64 decode a std::string.
        if (_tag != tag_type::bstring) {
            throw std::domain_error(std::format("Can't convert {} to an bstring", repr(*this)));
        }
        return get<bstring>(*this);
    }

    [[nodiscard]] constexpr char const *type_name() const noexcept
    {
        switch (_tag) {
        case tag_type::floating_point: return "float";
        case tag_type::decimal: return "decimal";
        case tag_type::integral: return "int";
        case tag_type::boolean: return "bool";
        case tag_type::year_month_day: return "date";
        case tag_type::string: return "string";
        case tag_type::url: return "url";
        case tag_type::vector: return "vector";
        case tag_type::map: return "map";
        case tag_type::bstring: return "bytes";
        default: tt_no_default();
        }
    }

    /** Check if the datum has an undefined value.
     */
    [[nodiscard]] constexpr bool is_undefined() const noexcept
    {
        return _tag == tag_type::monostate;
    }

    /** Check if the result of a expression was a break flow control statement.
     * @see skeleton_node
     */
    [[nodiscard]] constexpr bool is_break() const noexcept
    {
        return _tag == tag_type::flow_break;
    }

    /** Check if the result of a expression was a continue flow control statement.
     * @see skeleton_node
     */
    [[nodiscard]] constexpr bool is_continue() const noexcept
    {
        return _tag == tag_type::flow_continue;
    }

    [[nodiscard]] constexpr size_t hash() const noexcept
    {
        switch (_tag) {
        case tag_type::floating_point: return std::hash<double>{}(_value._double);
        case tag_type::decimal: return std::hash<decimal>{}(_value._decimal);
        case tag_type::integral: return std::hash<long long>{}(_value._long_long);
        case tag_type::boolean: return std::hash<bool>{}(_value._bool);
        case tag_type::year_month_day: {
            uint32_t r = 0;
            r |= static_cast<uint32_t>(static_cast<int>(_value._year_month_day.year())) << 16;
            r |= static_cast<uint32_t>(static_cast<unsigned>(_value._year_month_day.month())) << 8;
            r |= static_cast<uint32_t>(static_cast<unsigned>(_value._year_month_day.day()));
            return std::hash<uint32_t>{}(r);
        }
        case tag_type::string: return std::hash<std::string>{}(*_value._string);
        case tag_type::vector: {
            size_t r = 0;
            for (ttlet &v : *_value._vector) {
                r = hash_mix(r, v.hash());
            }
            return r;
        }
        case tag_type::map: {
            size_t r = 0;
            for (ttlet &kv : *_value._map) {
                r = hash_mix(r, kv.first.hash(), kv.second.hash());
            }
            return r;
        }
        case tag_type::url: return std::hash<URL>{}(*_value._url);
        case tag_type::bstring: return std::hash<bstring>{}(*_value._bstring);
        default: tt_no_default();
        }
    }

    [[nodiscard]] constexpr size_t size() const
    {
        if (ttlet *s = get_if<std::string>(*this)) {
            return std::size(*s);
        } else if (ttlet *v = get_if<vector_type>(*this)) {
            return std::size(*v);
        } else if (ttlet *m = get_if<map_type>(*this)) {
            return std::size(*m);
        } else if (ttlet *b = get_if<bstring>(*this)) {
            return std::size(get<bstring>(*this));
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.size()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr datum const &back() const
    {
        if (ttlet *v = get_if<vector_type>(*this)) {
            if (v->empty()) {
                throw std::domain_error(std::format("Empty vector {}.back()", repr(*this)));
            }
            return v->back();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.back()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr datum &back()
    {
        if (auto *v = get_if<vector_type>(*this)) {
            if (v->empty()) {
                throw std::domain_error(std::format("Empty vector {}.back()", repr(*this)));
            }
            return v->back();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.back()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr datum const &front() const
    {
        if (ttlet *v = get_if<vector_type>(*this)) {
            if (v->empty()) {
                throw std::domain_error(std::format("Empty vector {}.front()", repr(*this)));
            }
            return v->front();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.front()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr datum &front()
    {
        if (auto *v = get_if<vector_type>(*this)) {
            if (v->empty()) {
                throw std::domain_error(std::format("Empty vector {}.front()", repr(*this)));
            }
            return v->front();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.front()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr auto cbegin() const
    {
        if (ttlet *v = get_if<vector_type>(*this)) {
            return v->cbegin();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.cbegin()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr auto begin() const
    {
        if (ttlet *v = get_if<vector_type>(*this)) {
            return v->begin();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.begin()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr auto begin()
    {
        if (ttlet *v = get_if<vector_type>(*this)) {
            return v->begin();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.begin()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr auto cend() const
    {
        if (ttlet *v = get_if<vector_type>(*this)) {
            return v->cend();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.cend()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr auto end() const
    {
        if (ttlet *v = get_if<vector_type>(*this)) {
            return v->end();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.end()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr auto end()
    {
        if (ttlet *v = get_if<vector_type>(*this)) {
            return v->end();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.end()", repr(*this)));
        }
    }

    /** Get the sorted list of keys of a map.
     */
    [[nodiscard]] constexpr vector_type keys() const
    {
        if (ttlet *m = get_if<map_type>(*this)) {
            auto r = vector_type{};
            r.reserve(std::size(*m));
            for (ttlet &kv : *m) {
                r.push_back(kv.first);
            }
            std::sort(r.begin(), r.end());
            return r;
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.keys()", repr(*this)));
        }
    }

    /** Get the list of values of a map.
     */
    [[nodiscard]] constexpr vector_type values() const
    {
        if (ttlet *m = get_if<map_type>(*this)) {
            auto r = vector_type{};
            r.reserve(std::size(*m));
            for (ttlet &kv : *m) {
                r.push_back(kv.second);
            }
            return r;
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.values()", repr(*this)));
        }
    }

    /** Get key value pairs of items of a map sorted by the key.
     */
    [[nodiscard]] constexpr vector_type items() const
    {
        if (ttlet *m = get_if<map_type>(*this)) {
            auto r = vector_type{};
            r.reserve(std::size(*m));

            ttlet keys_ = keys();
            for (ttlet &key : keys_) {
                auto it = m->find(key);
                tt_axiom(it != m->end());
                r.push_back(make_vector(it->first, it->second));
            }
            return r;
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.items()", repr(*this)));
        }
    }

    constexpr void push_back(datum const &rhs)
    {
        if (auto *v = get_if<vector_type>(*this)) {
            return v->push_back(rhs);
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.push_back({})", repr(*this), repr(rhs)));
        }
    }

    constexpr void push_back(datum &&rhs)
    {
        if (auto *v = get_if<vector_type>(*this)) {
            return v->push_back(std::move(rhs));
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.push_back({})", repr(*this), repr(rhs)));
        }
    }

    template<typename Arg>
    constexpr void push_back(Arg &&arg)
    {
        push_back(datum{std::forward<Arg>(arg)});
    }

    constexpr void pop_back()
    {
        if (auto *v = get_if<vector_type>(*this)) {
            if (v->empty()) {
                throw std::domain_error(std::format("Empty vector {}.pop_back()", repr(*this)));
            }
            return v->pop_back();
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.pop_back()", repr(*this)));
        }
    }

    [[nodiscard]] constexpr bool contains(datum const &rhs) const
    {
        if (auto *m = get_if<map_type>(*this)) {
            return m->contains(rhs);
        } else {
            throw std::domain_error(std::format("Can not evaluate {}.contains({})", repr(*this), repr(rhs)));
        }
    }

    template<typename Arg>
    [[nodiscard]] constexpr bool contains(Arg &&arg) const
    {
        return contains(datum{std::forward<Arg>(arg)});
    }

    [[nodiscard]] constexpr datum const &operator[](datum const &rhs) const
    {
        if (holds_alternative<vector_type>(*this) and holds_alternative<long long>(rhs)) {
            ttlet &v = get<vector_type>(*this);

            auto index = get<long long>(rhs);
            if (index < 0) {
                index = std::ssize(v) + index;
            }
            if (index < 0 or index >= std::ssize(v)) {
                throw std::overflow_error(std::format("Index {} beyond bounds of vector", repr(rhs)));
            }

            return v[index];

        } else if (holds_alternative<map_type>(*this)) {
            ttlet &m = get<map_type>(*this);
            ttlet it = m.find(rhs);
            if (it == m.end()) {
                throw std::overflow_error(std::format("Key {} not found in map", repr(rhs)));
            }

            return it->second;

        } else {
            throw std::domain_error(std::format("Can not evaluate {}[{}]", repr(*this), repr(rhs)));
        }
    }

    [[nodiscard]] constexpr datum &operator[](datum const &rhs)
    {
        if (holds_alternative<vector_type>(*this) and holds_alternative<long long>(rhs)) {
            auto &v = get<vector_type>(*this);

            auto index = get<long long>(rhs);
            if (index < 0) {
                index = std::ssize(v) + index;
            }
            if (index < 0 or index >= std::ssize(v)) {
                throw std::overflow_error(std::format("Index {} beyond bounds of vector", repr(rhs)));
            }

            return v[index];

        } else if (holds_alternative<map_type>(*this)) {
            auto &m = get<map_type>(*this);
            return m[rhs];

        } else {
            throw std::domain_error(std::format("Can not evaluate {}[{}]", repr(*this), repr(rhs)));
        }
    }

    [[nodiscard]] constexpr datum const &operator[](auto const &rhs) const
    {
        return (*this)[datum{rhs}];
    }

    [[nodiscard]] constexpr datum &operator[](auto const &rhs)
    {
        return (*this)[datum{rhs}];
    }

    [[nodiscard]] constexpr datum &operator++()
    {
        if (holds_alternative<long long>(*this)) {
            ++_value._long_long;
            return *this;
        } else {
            throw std::domain_error(std::format("Can not evaluate ++{}", repr(*this)));
        }
    }

    [[nodiscard]] constexpr datum &operator--()
    {
        if (holds_alternative<long long>(*this)) {
            --_value._long_long;
            return *this;
        } else {
            throw std::domain_error(std::format("Can not evaluate --{}", repr(*this)));
        }
    }

    [[nodiscard]] constexpr datum operator++(int)
    {
        if (holds_alternative<long long>(*this)) {
            auto tmp = *this;
            _value._long_long++;
            return tmp;
        } else {
            throw std::domain_error(std::format("Can not evaluate {}++", repr(*this)));
        }
    }
    [[nodiscard]] constexpr datum operator--(int)
    {
        if (holds_alternative<long long>(*this)) {
            auto tmp = *this;
            _value._long_long--;
            return tmp;
        } else {
            throw std::domain_error(std::format("Can not evaluate {}--", repr(*this)));
        }
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
        if (ttlet doubles = promote_if<double>(lhs, rhs)) {
            return doubles.lhs() == doubles.rhs();

        } else if (ttlet decimals = promote_if<decimal>(lhs, rhs)) {
            return decimals.lhs() == decimals.rhs();

        } else if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            return long_longs.lhs() == long_longs.rhs();

        } else if (ttlet bools = promote_if<bool>(lhs, rhs)) {
            return bools.lhs() == bools.rhs();

        } else if (ttlet ymds = promote_if<std::chrono::year_month_day>(lhs, rhs)) {
            return ymds.lhs() == ymds.rhs();

        } else if (ttlet urls = promote_if<URL>(lhs, rhs)) {
            return urls.lhs() == urls.rhs();

        } else if (ttlet strings = promote_if<std::string>(lhs, rhs)) {
            return strings.lhs() == strings.rhs();

        } else if (ttlet vectors = promote_if<vector_type>(lhs, rhs)) {
            return vectors.lhs() == vectors.rhs();

        } else if (ttlet maps = promote_if<map_type>(lhs, rhs)) {
            return maps.lhs() == maps.rhs();

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
    [[nodiscard]] friend constexpr std::partial_ordering operator<=>(datum const &lhs, datum const &rhs) noexcept
    {
        if (ttlet doubles = promote_if<double>(lhs, rhs)) {
            return doubles.lhs() <=> doubles.rhs();

        } else if (ttlet decimals = promote_if<decimal>(lhs, rhs)) {
            return decimals.lhs() <=> decimals.rhs();

        } else if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            return long_longs.lhs() <=> long_longs.rhs();

        } else if (ttlet bools = promote_if<bool>(lhs, rhs)) {
            return bools.lhs() <=> bools.rhs();

        } else if (ttlet year_month_days = promote_if<std::chrono::year_month_day>(lhs, rhs)) {
            return year_month_days.lhs() <=> year_month_days.rhs();

        } else if (ttlet urls = promote_if<URL>(lhs, rhs)) {
            return urls.lhs() <=> urls.rhs();

        } else if (ttlet strings = promote_if<std::string>(lhs, rhs)) {
            return strings.lhs() <=> strings.rhs();

        } else if (ttlet vectors = promote_if<vector_type>(lhs, rhs)) {
            return vectors.lhs() <=> vectors.rhs();

            //} else if (ttlet maps = promote_if<map_type>(lhs, rhs)) {
            //    ttlet lhs_keys = maps.lhs().keys();
            //    ttlet rhs_keys = maps.rhs().keys();
            //    auto lhs_it = lhs_keys.begin();
            //    auto rhs_it = rhs_keys.begin();
            //    while (lhs_it != lhs_keys.end() and rhs_it != rhs_keys.end()) {
            //        ttlet key_cmp = *lhs_it <=> *rhs_it;
            //        if (key_cmp != std::partial_ordering::equal) {
            //            return key_cmp;
            //        }
            //
            //        ttlet &lhs_value = maps.lhs()[*lhs_key];
            //        ttlet &rhs_value = maps.rhs()[*rhs_key];
            //        ttlet value_cmp = lhs_value <=> rhs_value;
            //        if (value_cmp != std::partial_ordering::equal) {
            //            return value_cmp;
            //        }
            //
            //        ++lhs_it;
            //        ++rhs_it;
            //    }
            //    if (lhs != lhs_keys.end()) {
            //        return std::partial_ordering::greater;
            //    } else if (rhs != rhs_keys.end()) {
            //        return std::partial_ordering::smaller;
            //    } else {
            //        return std::partial_ordering::equal;
            //    }
        } else {
            return lhs._tag <=> rhs._tag;
        }
    }

    [[nodiscard]] friend constexpr datum operator-(datum const &rhs)
    {
        if (ttlet rhs_double = get_if<double>(rhs)) {
            return datum{-*rhs_double};

        } else if (ttlet rhs_decimal = get_if<decimal>(rhs)) {
            return datum{-*rhs_decimal};

        } else if (ttlet rhs_long_long = get_if<long long>(rhs)) {
            return datum{-*rhs_long_long};

        } else {
            throw std::domain_error(std::format("Can not evaluate -{}", repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator~(datum const &rhs)
    {
        if (ttlet rhs_long_long = get_if<long long>(rhs)) {
            return datum{-*rhs_long_long};

        } else {
            throw std::domain_error(std::format("Can not evaluate ~{}", repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator+(datum const &lhs, datum const &rhs)
    {
        if (ttlet doubles = promote_if<double>(lhs, rhs)) {
            return datum{doubles.lhs() + doubles.rhs()};

        } else if (ttlet decimals = promote_if<decimal>(lhs, rhs)) {
            return datum{decimals.lhs() + decimals.rhs()};

        } else if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            return datum{long_longs.lhs() + long_longs.rhs()};

        } else if (ttlet strings = promote_if<std::string>(lhs, rhs)) {
            return datum{strings.lhs() + strings.rhs()};

        } else if (ttlet vectors = promote_if<vector_type>(lhs, rhs)) {
            auto r = vectors.lhs();
            r.insert(r.end(), vectors.rhs().begin(), vectors.rhs().end());
            return datum{std::move(r)};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '+' {}", repr(lhs), repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator-(datum const &lhs, datum const &rhs)
    {
        if (ttlet doubles = promote_if<double>(lhs, rhs)) {
            return datum{doubles.lhs() - doubles.rhs()};

        } else if (ttlet decimals = promote_if<decimal>(lhs, rhs)) {
            return datum{decimals.lhs() - decimals.rhs()};

        } else if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            return datum{long_longs.lhs() - long_longs.rhs()};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '-' {}", repr(lhs), repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator*(datum const &lhs, datum const &rhs)
    {
        if (ttlet doubles = promote_if<double>(lhs, rhs)) {
            return datum{doubles.lhs() * doubles.rhs()};

        } else if (ttlet decimals = promote_if<decimal>(lhs, rhs)) {
            return datum{decimals.lhs() * decimals.rhs()};

        } else if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            return datum{long_longs.lhs() * long_longs.rhs()};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '*' {}", repr(lhs), repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator/(datum const &lhs, datum const &rhs)
    {
        if (ttlet doubles = promote_if<double>(lhs, rhs)) {
            if (doubles.rhs() == 0) {
                throw std::domain_error(std::format("Divide by zero {} '/' {}", repr(lhs), repr(rhs)));
            }
            return datum{doubles.lhs() / doubles.rhs()};

        } else if (ttlet decimals = promote_if<decimal>(lhs, rhs)) {
            if (decimals.rhs() == 0) {
                throw std::domain_error(std::format("Divide by zero {} '/' {}", repr(lhs), repr(rhs)));
            }
            return datum{decimals.lhs() / decimals.rhs()};

        } else if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            if (long_longs.rhs() == 0) {
                throw std::domain_error(std::format("Divide by zero {} '/' {}", repr(lhs), repr(rhs)));
            }
            return datum{long_longs.lhs() / long_longs.rhs()};

        } else if (ttlet urls = promote_if<URL>(lhs, rhs)) {
            return datum{urls.lhs() / urls.rhs()};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '/' {}", repr(lhs), repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator%(datum const &lhs, datum const &rhs)
    {
        if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            if (long_longs.rhs() == 0) {
                throw std::domain_error(std::format("Divide by zero {} '%' {}", repr(lhs), repr(rhs)));
            }
            return datum{long_longs.lhs() % long_longs.rhs()};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '%' {}", repr(lhs), repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator&(datum const &lhs, datum const &rhs)
    {
        if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            return datum{long_longs.lhs() & long_longs.rhs()};

        } else if (ttlet bools = promote_if<bool>(lhs, rhs)) {
            return datum{bools.lhs() and bools.rhs()};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '&' {}", repr(lhs), repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator|(datum const &lhs, datum const &rhs)
    {
        if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            return datum{long_longs.lhs() | long_longs.rhs()};

        } else if (ttlet bools = promote_if<bool>(lhs, rhs)) {
            return datum{bools.lhs() or bools.rhs()};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '|' {}", repr(lhs), repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator^(datum const &lhs, datum const &rhs)
    {
        if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            return datum{long_longs.lhs() ^ long_longs.rhs()};

        } else if (ttlet bools = promote_if<bool>(lhs, rhs)) {
            return datum{bools.lhs() != bools.rhs()};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '^' {}", repr(lhs), repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator<<(datum const &lhs, datum const &rhs)
    {
        if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            if (long_longs.rhs() < 0 or long_longs.rhs() > (sizeof(long long) * CHAR_BIT - 1)) {
                throw std::domain_error(std::format("Invalid shift count {} '<<' {}", repr(lhs), repr(rhs)));
            }
            return datum{long_longs.lhs() << long_longs.rhs()};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '<<' {}", repr(lhs), repr(rhs)));
        }
    }

    [[nodiscard]] friend constexpr datum operator>>(datum const &lhs, datum const &rhs)
    {
        if (ttlet long_longs = promote_if<long long>(lhs, rhs)) {
            if (long_longs.rhs() < 0 or long_longs.rhs() > (sizeof(long long) * CHAR_BIT - 1)) {
                throw std::domain_error(std::format("Invalid shift count {} '>>' {}", repr(lhs), repr(rhs)));
            }
            return datum{long_longs.lhs() >> long_longs.rhs()};

        } else {
            throw std::domain_error(std::format("Can not evaluate {} '>>' {}", repr(lhs), repr(rhs)));
        }
    }

    friend std::ostream &operator<<(std::ostream &lhs, datum const &rhs)
    {
        return lhs << to_string(rhs);
    }

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
    X(^) X(<<) X(>>)
#undef X

        /** Get the string representation of the value.
         */
        [[nodiscard]] friend std::string repr(datum const &rhs) noexcept
    {
        // XXX strings need quotes.
        return static_cast<std::string>(rhs);
    }

    /** Get the string representation of the value.
     */
    [[nodiscard]] friend std::string to_string(datum const &rhs) noexcept
    {
        return static_cast<std::string>(rhs);
    }

    /** Check if the datum holds a type.
     *
     * @tparam T Type to check.
     * @param rhs The datum to check the value-type of.
     * @return True if the value-type matches the template parameter @a T
     */
    template<typename T>
    [[nodiscard]] friend constexpr bool holds_alternative(datum const &rhs) noexcept
    {
        if constexpr (std::is_same_v<T, double>) {
            return rhs._tag == tag_type::floating_point;
        } else if constexpr (std::is_same_v<T, decimal>) {
            return rhs._tag == tag_type::decimal;
        } else if constexpr (std::is_same_v<T, long long>) {
            return rhs._tag == tag_type::integral;
        } else if constexpr (std::is_same_v<T, bool>) {
            return rhs._tag == tag_type::boolean;
        } else if constexpr (std::is_same_v<T, std::chrono::year_month_day>) {
            return rhs._tag == tag_type::year_month_day;
        } else if constexpr (std::is_same_v<T, nullptr_t>) {
            return rhs._tag == tag_type::null;
        } else if constexpr (std::is_same_v<T, std::monostate>) {
            return rhs._tag == tag_type::monostate;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return rhs._tag == tag_type::string;
        } else if constexpr (std::is_same_v<T, vector_type>) {
            return rhs._tag == tag_type::vector;
        } else if constexpr (std::is_same_v<T, map_type>) {
            return rhs._tag == tag_type::map;
        } else if constexpr (std::is_same_v<T, URL>) {
            return rhs._tag == tag_type::url;
        } else if constexpr (std::is_same_v<T, bstring>) {
            return rhs._tag == tag_type::bstring;
        } else {
            tt_static_no_default();
        }
    }

    /** Check if the type held by the datum can be promoted.
     */
    template<typename T>
    [[nodiscard]] friend constexpr bool promotable_to(datum const &rhs) noexcept
    {
        if constexpr (std::is_same_v<T, double>) {
            return holds_alternative<double>(rhs) or holds_alternative<decimal>(rhs) or holds_alternative<long long>(rhs) or
                holds_alternative<bool>(rhs);
        } else if constexpr (std::is_same_v<T, decimal>) {
            return holds_alternative<decimal>(rhs) or holds_alternative<long long>(rhs) or holds_alternative<bool>(rhs);
        } else if constexpr (std::is_same_v<T, long long>) {
            return holds_alternative<long long>(rhs) or holds_alternative<bool>(rhs);
        } else if constexpr (std::is_same_v<T, bool>) {
            return holds_alternative<bool>(rhs);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return holds_alternative<URL>(rhs) or holds_alternative<std::string>(rhs);
        } else if constexpr (std::is_same_v<T, URL>) {
            return holds_alternative<URL>(rhs) or holds_alternative<std::string>(rhs);
        } else {
            return holds_alternative<T>(rhs);
        }
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
    [[nodiscard]] friend constexpr T const &get(datum const &rhs) noexcept
    {
        tt_axiom(holds_alternative<T>(rhs));
        if constexpr (std::is_same_v<T, double>) {
            return rhs._value._double;
        } else if constexpr (std::is_same_v<T, decimal>) {
            return rhs._value._decimal;
        } else if constexpr (std::is_same_v<T, long long>) {
            return rhs._value._long_long;
        } else if constexpr (std::is_same_v<T, bool>) {
            return rhs._value._bool;
        } else if constexpr (std::is_same_v<T, std::chrono::year_month_day>) {
            return rhs._value._year_month_day;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return *rhs._value._string;
        } else if constexpr (std::is_same_v<T, vector_type>) {
            return *rhs._value._vector;
        } else if constexpr (std::is_same_v<T, map_type>) {
            return *rhs._value._map;
        } else if constexpr (std::is_same_v<T, URL>) {
            return *rhs._value._url;
        } else if constexpr (std::is_same_v<T, bstring>) {
            return *rhs._value._bstring;
        } else {
            tt_static_no_default();
        }
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
    [[nodiscard]] friend constexpr T &get(datum &rhs) noexcept
    {
        tt_axiom(holds_alternative<T>(rhs));
        if constexpr (std::is_same_v<T, double>) {
            return rhs._value._double;
        } else if constexpr (std::is_same_v<T, decimal>) {
            return rhs._value._decimal;
        } else if constexpr (std::is_same_v<T, long long>) {
            return rhs._value._long_long;
        } else if constexpr (std::is_same_v<T, bool>) {
            return rhs._value._bool;
        } else if constexpr (std::is_same_v<T, std::chrono::year_month_day>) {
            return rhs._value._year_month_day;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return *rhs._value._string;
        } else if constexpr (std::is_same_v<T, vector_type>) {
            return *rhs._value._vector;
        } else if constexpr (std::is_same_v<T, map_type>) {
            return *rhs._value._map;
        } else if constexpr (std::is_same_v<T, URL>) {
            return *rhs._value._url;
        } else if constexpr (std::is_same_v<T, bstring>) {
            return *rhs._value._bstring;
        } else {
            tt_static_no_default();
        }
    }

    /** Get the value of a datum.
     *
     * @tparam T Type to check.
     * @param rhs The datum to get the value from.
     * @return A pointer to the value, or nullptr.
     */
    template<typename T>
    [[nodiscard]] friend constexpr T *get_if(datum &rhs) noexcept
    {
        if (holds_alternative<T>(rhs)) {
            return &get<T>(rhs);
        } else {
            return nullptr;
        }
    }

    template<typename T>
    [[nodiscard]] friend constexpr T const *get_if(datum const &rhs) noexcept
    {
        if (holds_alternative<T>(rhs)) {
            return &get<T>(rhs);
        } else {
            return nullptr;
        }
    }

    /** Promote two datum-arguments to a common type.
     *
     * @tparam To Type to promote to.
     * @tparam LHS Type which has the `holds_alternative()` and `get()` free functions
     * @tparam RHS Type which has the `holds_alternative()` and `get()` free functions
     * @param lhs The left hand side.
     * @param rhs The right hand side.
     */
    template<typename To>
    [[nodiscard]] friend constexpr auto promote_if(datum const &lhs, datum const &rhs) noexcept
    {
        auto r = detail::datum_promotion_result<To>{};
        if (holds_alternative<To>(lhs) and holds_alternative<To>(rhs)) {
            r.set(get<To>(lhs), get<To>(rhs));

        } else if (holds_alternative<To>(lhs) and promotable_to<To>(rhs)) {
            r.set(get<To>(lhs), static_cast<To>(rhs));

        } else if (promotable_to<To>(lhs) and holds_alternative<To>(rhs)) {
            r.set(static_cast<To>(lhs), get<To>(rhs));
        }

        return r;
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

    tag_type _tag = tag_type::monostate;
    union value_type {
        double _double;
        long long _long_long;
        decimal _decimal;
        bool _bool;
        std::chrono::year_month_day _year_month_day;
        std::string *_string;
        vector_type *_vector;
        map_type *_map;
        URL *_url;
        bstring *_bstring;

        constexpr value_type(numeric_integral auto value) noexcept : _long_long(static_cast<long long>(value)) {}
        constexpr value_type(std::floating_point auto value) noexcept : _double(static_cast<double>(value)) {}
        constexpr value_type(decimal value) noexcept : _decimal(value) {}
        constexpr value_type(bool value) noexcept : _bool(value) {}
        constexpr value_type(std::chrono::year_month_day value) noexcept : _year_month_day(value) {}
        constexpr value_type(std::string *value) noexcept : _string(value) {}
        constexpr value_type(vector_type *value) noexcept : _vector(value) {}
        constexpr value_type(map_type *value) noexcept : _map(value) {}
        constexpr value_type(URL *value) noexcept : _url(value) {}
        constexpr value_type(bstring *value) noexcept : _bstring(value) {}
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
        case tag_type::string: _value._string = new std::string{*other._value._string}; return;
        case tag_type::vector: _value._vector = new vector_type{*other._value._vector}; return;
        case tag_type::map: _value._map = new map_type{*other._value._map}; return;
        case tag_type::url: _value._url = new URL{*other._value._url}; return;
        case tag_type::bstring: _value._bstring = new bstring{*other._value._bstring}; return;
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
};

} // namespace tt

namespace std {
[[nodiscard]] constexpr size_t hash<tt::datum>::operator()(tt::datum const &rhs) const noexcept
{
    return rhs.hash();
}

template<typename CharT>
struct std::formatter<tt::datum, CharT> : std::formatter<std::string_view, CharT> {
    auto format(tt::datum const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};

} // namespace std
