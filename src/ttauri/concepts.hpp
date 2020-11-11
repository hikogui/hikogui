// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "type_traits.hpp"
#include <type_traits>

namespace tt {

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept pointer = std::is_pointer_v<T>;

template<typename T>
concept reference = std::is_reference_v<T>;

template<typename T>
concept lvalue_reference = std::is_lvalue_reference_v<T>;

template<typename T>
concept rvalue_reference = std::is_rvalue_reference_v<T>;

template<typename BaseType, typename DerivedType>
concept base_of = std::is_base_of_v<BaseType,DerivedType>;

template<typename DerivedType, typename BaseType>
concept derived_from = tt::is_derived_from_v<DerivedType,BaseType>;

template<typename BaseType, typename DerivedType>
concept strict_base_of = base_of<BaseType,DerivedType> && ! std::same_as<BaseType,DerivedType>;

template<typename BaseType, typename DerivedType>
concept strict_derived_from = derived_from<BaseType, DerivedType> && !std::same_as<BaseType, DerivedType>;


}
