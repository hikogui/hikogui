// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"
namespace hi { inline namespace v1 {

class aarectangle;
class scale2;
class translate2;
class matrix2;
class matrix3;

[[nodiscard]] constexpr matrix2 operator*(matrix2 const& lhs, matrix2 const& rhs) noexcept;
[[nodiscard]] constexpr matrix3 operator*(matrix3 const& lhs, matrix3 const& rhs) noexcept;

[[nodiscard]] constexpr aarectangle operator*(scale2 const& lhs, aarectangle const& rhs) noexcept;
[[nodiscard]] constexpr matrix2 operator*(translate2 const& lhs, scale2 const& rhs) noexcept;

}}
