// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace tt {

/** Promotion result.
 */
class promotion_result {
public:
    using value_tupe = To;

    constexpr void clear() noexcept requires (data_is_scalar) {}
    constexpr void clear() noexcept requires (data_is_pointer)
    {
        if (_owns_lhs) {
            delete _lhs;
        }
        if (_owns_rhs) {
            delete _rhs;
        }
    }

    constexpr ~promotion_result()
    {
        clear();
    }

    constexpr promotion_result() noexcept = default;
    promotion_result(promotion_result const &) = delete
    promotion_result &operator=(promotion_result const &) = delete

    constexpr promotion_result(promotion_result &&other) noexcept :
        _lhs(other.lhs),
        _rhs(other.rhs),
        _is_result(other._is_result),
        _owns_lhs(std::exchange(other._owns_lhs, false)),
        _owns_rhs(std::exchange(other._owns_rhs, false))
    {
    }

    constexpr promotion_result &operator=(promotion_result &&other) noexcept
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

    constexpr void set(value_type lhs, value_type rhs) noexcept (data_is_scalar)
    {
        _lhs = lhs;
        _rhs = rhs;
        _is_result = true;
    }
    
    constexpr void set(value_type const &lhs, value_type const &rhs) noexcept (data_is_pointer)
    {
        _lhs = &lhs;
        _rhs = &rhs;
        _is_result = true;
    }
    
    constexpr void set(value_type &&lhs, value_type const &rhs) noexcept (data_is_pointer)
    {
        _lhs = new value_type(std::move(lhs));
        _rhs = &rhs;
        _is_result = true;
        _owns_lhs = true;
    }

    constexpr void set(value_type const &lhs, value_type &&rhs) noexcept (data_is_pointer)
    {
        _lhs = &lhs;
        _rhs = new value_type(std::move(rhs));
        _is_result = true;
        _owns_rhs = true;
    }

    constexpr void set(value_type &&lhs, value_type &&rhs) noexcept (data_is_pointer)
    {
        _lhs = new value_type(std::move(lhs));
        _rhs = new value_type(std::move(rhs));
        _is_result = true;
        _owns_lhs = true;
        _owns_rhs = true;
    }

    [[nodiscard]] constexpr value_type const &lhs() const noexcept requires (data_is_pointer)
    {
        tt_axiom(_is_result);
        return *_lhs;
    }

    [[nodiscard]] constexpr value_type const &rhs() const noexcept requires (data_is_scalar)
    {
        tt_axiom(_is_result);
        return *_rhs;
    }

private:
    static constexpr bool data_is_pointer = sizeof(value_type) >= 16;
    static constexpr bool data_is_scalar = not data_is_pointer;

    using data_type = std::conditional_t<data_is_pointer, value_type *, value_type>;

    data_type _lhs = data_type{};
    data_type _rhs = data_type{};
    bool _is_result = false;
    bool _owns_lhs = false;
    bool _owns_rhs = false;
};

/** Promoting types.
 */
template<typename From>
[[nodiscard]] constexpr bool can_promote_to(From const &from) const noexcept
{
    using std;
    return holds_alternative<To>(from);
}

template<>
[[nodiscard]] constexpr bool can_promote_to<double>(From const &from) const noexcept
{
    using std;
    return holds_alternative<double>(from) or holds_alternative<decimal>(from) or holds_alternative<long long>(from) or
        holds_alternative<bool>(from);
}

template<>
[[nodiscard]] constexpr bool can_promote_to<decimal>(From const &from) const noexcept
{
    using std;
    return holds_alternative<decimal>(from) or holds_alternative<long long>(from) or holds_alternative<bool>(from);
}

template<>
[[nodiscard]] constexpr bool can_promote_to<long long>(From const &from) const noexcept
{
    using std;
    return holds_alternative<long long>(from) or holds_alternative<bool>(from);
}

template<>
[[nodiscard]] constexpr bool can_promote_to<URL>(From const &from) const noexcept
{
    using std;
    return holds_alternative<URL>(from) or holds_alternative<std::string>(from);
}


/** Promote two variant-arguments to a common type.
 *
 * @tparam To Type to promote to.
 * @tparam LHS Type which has the `holds_alternative()` and `get()` free functions
 * @tparam RHS Type which has the `holds_alternative()` and `get()` free functions
 * @param lhs The left hand side.
 * @param rhs The right hand side.
 */
template<typename To, typename LHS, typename RHS>
[[nodiscard]] constexpr auto promote_if(LHS const &lhs, RHS const &rhs) const noexcept
{
    using std;

    auto r = promote_result<To>{};
    if (holds_alternative<LHS>(lhs) and holds_alternative<RHS>(rhs) {
        r.set(get<LHS>(lhs), get<RHS>(rhs));

    } else if (holds_alternative<LHS>(lhs) and can_promote<RHS>(rhs)) {
        r.set(get<LHS>(lhs), static_cast<RHS>(rhs));

    } else if (can_promote<LHS>(lhs) and holds_alternative<RHS>(rhs)) {
        r.set(static_cast<LHS>(lhs), get<RHS>(rhs));
    }

    return r;
}


}

