// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "type_traits.hpp"
#include <type_traits>
#include <string>
#include <concepts>

namespace tt {

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept pointer = std::is_pointer_v<T>;

template<typename T>
concept reference = std::is_reference_v<T>;

template<typename T, typename O>
concept same = std::is_same_v<T, O>;

template<typename T>
concept lvalue_reference = std::is_lvalue_reference_v<T>;

template<typename T>
concept rvalue_reference = std::is_rvalue_reference_v<T>;

template<typename BaseType, typename DerivedType>
concept base_of = std::is_base_of_v<BaseType, DerivedType>;

template<typename BaseType, typename DerivedType>
concept decayed_base_of = is_decayed_base_of_v<BaseType, DerivedType>;

template<typename DerivedType, typename BaseType>
concept derived_from = tt::is_derived_from_v<DerivedType, BaseType>;

template<typename DerivedType, typename BaseType>
concept decayed_derived_from = tt::is_decayed_derived_from_v<DerivedType, BaseType>;

template<typename BaseType, typename DerivedType>
concept strict_base_of = base_of<BaseType, DerivedType> && !std::same_as<BaseType, DerivedType>;

template<typename BaseType, typename DerivedType>
concept strict_derived_from = derived_from<BaseType, DerivedType> && !std::same_as<BaseType, DerivedType>;

template<typename T>
concept to_stringable = requires(T v)
{
    {
        to_string(v)
    }
    ->std::convertible_to<std::string>;
};

template<typename T>
concept from_stringable = requires()
{
    {
        from_string<T>(std::string_view{})
    }
    ->std::convertible_to<T>;
};

template<typename From, typename To>
concept static_castableable = requires(From v)
{
    {
        static_cast<To>(v)
    }
    ->std::convertible_to<To>;
};

template<typename T>
concept sizeable = requires(T v)
{
    {
        size(v)
    }
    ->std::convertible_to<size_t>;
};

} // namespace tt
