// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/string_tag.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include <limits>

namespace TTauri {

template<typename T, typename Friend, string_tag Tag, ssize_t Max=std::numeric_limits<T>::max() - 1>
class tagged_id {
public:
    static_assert(std::is_integral_v<T>, "Expecting tagged_id to be an integral");

    using type = T;
    constexpr static string_tag tag = Tag;
    constexpr static ssize_t max = Max;
    constexpr static ssize_t invalid = Max + 1;

private:
    type value;

    constexpr explicit tagged_id(signed long long rhs) noexcept : value(numeric_cast<type>(rhs)) {}
    constexpr explicit tagged_id(signed long rhs) noexcept : value(numeric_cast<type>(rhs)) {}
    constexpr explicit tagged_id(signed int rhs) noexcept : value(numeric_cast<type>(rhs)) {}
    constexpr explicit tagged_id(signed short rhs) noexcept : value(numeric_cast<type>(rhs)) {}
    constexpr explicit tagged_id(signed char rhs) noexcept : value(numeric_cast<type>(rhs)) {}
    constexpr explicit tagged_id(unsigned long long rhs) noexcept : value(numeric_cast<type>(rhs)) {}
    constexpr explicit tagged_id(unsigned long rhs) noexcept : value(numeric_cast<type>(rhs)) {}
    constexpr explicit tagged_id(unsigned int rhs) noexcept : value(numeric_cast<type>(rhs)) {}
    constexpr explicit tagged_id(unsigned short rhs) noexcept : value(numeric_cast<type>(rhs)) {}
    constexpr explicit tagged_id(unsigned char rhs) noexcept : value(numeric_cast<type>(rhs)) {}

    constexpr tagged_id &operator=(signed long long rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }
    constexpr tagged_id &operator=(signed long rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }
    constexpr tagged_id &operator=(signed int rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }
    constexpr tagged_id &operator=(signed short rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }
    constexpr tagged_id &operator=(signed char rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }
    constexpr tagged_id &operator=(unsigned long long rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }
    constexpr tagged_id &operator=(unsigned long rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }
    constexpr tagged_id &operator=(unsigned int rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }
    constexpr tagged_id &operator=(unsigned short rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }
    constexpr tagged_id &operator=(unsigned char rhs) noexcept { value = numeric_cast<type>(rhs); return *this; }

public:
    constexpr tagged_id() noexcept : value(invalid) {}
    constexpr tagged_id(tagged_id const &other) noexcept = default;
    constexpr tagged_id(tagged_id &&other) noexcept = default;
    constexpr tagged_id &operator=(tagged_id const &other) noexcept = default;
    constexpr tagged_id &operator=(tagged_id &&other) noexcept = default;

    operator signed long long () const noexcept { return numeric_cast<signed long long>(value); }
    operator signed long () const noexcept { return numeric_cast<signed long>(value); }
    operator signed int () const noexcept { return numeric_cast<signed int>(value); }
    operator signed short () const noexcept { return numeric_cast<signed short>(value); }
    operator signed char () const noexcept { return numeric_cast<signed char>(value); }
    operator unsigned long long () const noexcept { return numeric_cast<unsigned long long>(value); }
    operator unsigned long () const noexcept { return numeric_cast<unsigned long>(value); }
    operator unsigned int () const noexcept { return numeric_cast<unsigned int>(value); }
    operator unsigned short () const noexcept { return numeric_cast<unsigned short>(value); }
    operator unsigned char () const noexcept { return numeric_cast<unsigned char>(value); }

    operator bool () const noexcept { return value <= max; }

    template<typename O> [[nodiscard]] friend bool operator==(tagged_id const &lhs, O const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs.value == rhs; }
    template<typename O> [[nodiscard]] friend bool operator!=(tagged_id const &lhs, O const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs.value != rhs; }
    template<typename O> [[nodiscard]] friend bool operator<(tagged_id const &lhs, O const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs.value < rhs; }
    template<typename O> [[nodiscard]] friend bool operator>(tagged_id const &lhs, O const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs.value > rhs; }
    template<typename O> [[nodiscard]] friend bool operator<=(tagged_id const &lhs, O const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs.value <= rhs; }
    template<typename O> [[nodiscard]] friend bool operator>=(tagged_id const &lhs, O const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs.value >= rhs; }
    template<typename O> [[nodiscard]] friend bool operator==(O const &lhs, tagged_id const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs == rhs.value; }
    template<typename O> [[nodiscard]] friend bool operator!=(O const &lhs, tagged_id const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs != rhs.value; }
    template<typename O> [[nodiscard]] friend bool operator<(O const &lhs, tagged_id const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs < rhs.value; }
    template<typename O> [[nodiscard]] friend bool operator>(O const &lhs, tagged_id const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs > rhs.value; }
    template<typename O> [[nodiscard]] friend bool operator<=(O const &lhs, tagged_id const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs <= rhs.value; }
    template<typename O> [[nodiscard]] friend bool operator>=(O const &lhs, tagged_id const &rhs) noexcept { ttauri_assume(lhs.value <= max); return lhs >= rhs.value; }

    friend Friend;
};


};