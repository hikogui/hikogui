// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "concepts.hpp"
#include <type_traits>
#include <concepts>
#include <limits>

namespace tt {

template<typename T>
[[nodiscard]] T copy(T value)
{
    return value;
}

template<std::signed_integral OutType, std::floating_point InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    tt_assume(value >= std::numeric_limits<OutType>::lowest() && value <= std::numeric_limits<OutType>::max());
    return static_cast<OutType>(value);
}

template<std::signed_integral OutType, std::signed_integral InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    constexpr auto smin = static_cast<long long>(std::numeric_limits<OutType>::lowest());
    constexpr auto smax = static_cast<long long>(std::numeric_limits<OutType>::max());
    tt_assume(value >= smin && value <= smax);
    return static_cast<OutType>(value);
}

template<std::signed_integral OutType, std::unsigned_integral InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    constexpr auto umax = static_cast<unsigned long long>(std::numeric_limits<OutType>::max());
    tt_assume(value <= umax);
    return static_cast<OutType>(value);
}

template<std::unsigned_integral OutType, std::floating_point InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    tt_assume(value >= InType{0});
    tt_assume(value <= std::numeric_limits<OutType>::max());
    return static_cast<OutType>(value);
}

template<std::unsigned_integral OutType, std::signed_integral InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    tt_assume(value >= InType{0});
    if constexpr (sizeof(OutType) < sizeof(InType)) {
        constexpr auto smax = static_cast<long long>(std::numeric_limits<OutType>::max());
        tt_assume(value <= smax);
    }
    return static_cast<OutType>(value);
}

template<std::unsigned_integral OutType, std::unsigned_integral InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    constexpr auto umax = static_cast<unsigned long long>(std::numeric_limits<OutType>::max());
    tt_assume(value <= umax);
    return static_cast<OutType>(value);
}

template<std::floating_point OutType, tt::arithmetic InType>
[[nodiscard]] constexpr OutType narrow_cast(InType value) noexcept
{
    return static_cast<OutType>(value);
}

template<tt::lvalue_reference BaseType, tt::derived_from<std::remove_reference_t<BaseType>> DerivedType>
[[nodiscard]] BaseType narrow_cast(DerivedType &value)
{
    return static_cast<BaseType>(value);
}

template<tt::lvalue_reference DerivedType, tt::strict_base_of<std::remove_reference_t<DerivedType>> BaseType>
[[nodiscard]] DerivedType narrow_cast(BaseType &value)
{
    tt_assume(dynamic_cast<std::remove_reference_t<DerivedType> *>(&value) != nullptr);
    return static_cast<DerivedType>(value);
}

template<tt::pointer BaseType, tt::derived_from<std::remove_pointer_t<BaseType>> DerivedType>
[[nodiscard]] BaseType narrow_cast(DerivedType *value)
{
    return static_cast<BaseType>(value);
}

template<tt::pointer DerivedType, tt::strict_base_of<std::remove_pointer_t<DerivedType>> BaseType>
[[nodiscard]] DerivedType narrow_cast(BaseType *value)
{
    tt_assume(dynamic_cast<DerivedType>(value) != nullptr);
    return static_cast<DerivedType>(value);
}

} // namespace tt
