// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/string_tag.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/math.hpp"
#include <limits>
#include <typeinfo>
#include <typeindex>

namespace tt {

template<typename T, typename Tag, ssize_t Max = std::numeric_limits<T>::max() - 1>
class tagged_id {
public:
    static_assert(std::is_integral_v<T>, "Expecting tagged_id to be an integral");
    static_assert(Max < std::numeric_limits<T>::max(), "Max must be at least one less than the maximum value of T");

    using type = T;
    using TAG = Tag;

    constexpr static type max = Max;
    constexpr static type invalid = max + 1;

    constexpr static type mask = make_mask(invalid);

private:
    type value;

public:
    constexpr explicit tagged_id(signed long long rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }
    constexpr explicit tagged_id(signed long rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }
    constexpr explicit tagged_id(signed int rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }
    constexpr explicit tagged_id(signed short rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }
    constexpr explicit tagged_id(signed char rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }
    constexpr explicit tagged_id(unsigned long long rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }
    constexpr explicit tagged_id(unsigned long rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }
    constexpr explicit tagged_id(unsigned int rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }
    constexpr explicit tagged_id(unsigned short rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }
    constexpr explicit tagged_id(unsigned char rhs) noexcept : value(numeric_cast<type>(rhs)) { ttauri_assume(value <= invalid); }

    constexpr tagged_id &operator=(signed long long rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }
    constexpr tagged_id &operator=(signed long rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }
    constexpr tagged_id &operator=(signed int rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }
    constexpr tagged_id &operator=(signed short rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }
    constexpr tagged_id &operator=(signed char rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }
    constexpr tagged_id &operator=(unsigned long long rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }
    constexpr tagged_id &operator=(unsigned long rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }
    constexpr tagged_id &operator=(unsigned int rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }
    constexpr tagged_id &operator=(unsigned short rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }
    constexpr tagged_id &operator=(unsigned char rhs) noexcept { value = numeric_cast<type>(rhs); ttauri_assume(value <= invalid); return *this; }

    constexpr tagged_id() noexcept : value(invalid) {}
    constexpr tagged_id(tagged_id const &other) noexcept = default;
    constexpr tagged_id(tagged_id &&other) noexcept = default;
    constexpr tagged_id &operator=(tagged_id const &other) noexcept = default;
    constexpr tagged_id &operator=(tagged_id &&other) noexcept = default;

    constexpr operator signed long long () const noexcept { return numeric_cast<signed long long>(value); }
    constexpr operator signed long () const noexcept { return numeric_cast<signed long>(value); }
    constexpr operator signed int () const noexcept { return numeric_cast<signed int>(value); }
    constexpr operator signed short () const noexcept { return numeric_cast<signed short>(value); }
    constexpr operator signed char () const noexcept { return numeric_cast<signed char>(value); }
    constexpr operator unsigned long long () const noexcept { return numeric_cast<unsigned long long>(value); }
    constexpr operator unsigned long () const noexcept { return numeric_cast<unsigned long>(value); }
    constexpr operator unsigned int () const noexcept { return numeric_cast<unsigned int>(value); }
    constexpr operator unsigned short () const noexcept { return numeric_cast<unsigned short>(value); }
    constexpr operator unsigned char () const noexcept { return numeric_cast<unsigned char>(value); }

    constexpr operator bool () const noexcept { return value <= max; }

    [[nodiscard]] constexpr size_t hash() const noexcept { return std::hash<type>{}(value); }

    [[nodiscard]] constexpr friend bool operator==(tagged_id const &lhs, tagged_id const &rhs) noexcept { return lhs.value == rhs.value; }
    [[nodiscard]] constexpr friend bool operator!=(tagged_id const &lhs, tagged_id const &rhs) noexcept { return lhs.value != rhs.value; }
    [[nodiscard]] constexpr friend bool operator<(tagged_id const &lhs, tagged_id const &rhs) noexcept { return lhs.value < rhs.value; }
    [[nodiscard]] constexpr friend bool operator>(tagged_id const &lhs, tagged_id const &rhs) noexcept { return lhs.value > rhs.value; }
    [[nodiscard]] constexpr friend bool operator<=(tagged_id const &lhs, tagged_id const &rhs) noexcept { return lhs.value <= rhs.value; }
    [[nodiscard]] constexpr friend bool operator>=(tagged_id const &lhs, tagged_id const &rhs) noexcept { return lhs.value >= rhs.value; }

    template<typename O> [[nodiscard]] constexpr friend bool operator==(tagged_id const &lhs, O const &rhs) noexcept { return lhs == tagged_id{rhs}; }
    template<typename O> [[nodiscard]] constexpr friend bool operator!=(tagged_id const &lhs, O const &rhs) noexcept { return lhs != tagged_id{rhs}; }
    template<typename O> [[nodiscard]] constexpr friend bool operator<(tagged_id const &lhs, O const &rhs) noexcept { return lhs < tagged_id{rhs}; }
    template<typename O> [[nodiscard]] constexpr friend bool operator>(tagged_id const &lhs, O const &rhs) noexcept { return lhs > tagged_id{rhs}; }
    template<typename O> [[nodiscard]] constexpr friend bool operator<=(tagged_id const &lhs, O const &rhs) noexcept { return lhs <= tagged_id{rhs}; }
    template<typename O> [[nodiscard]] constexpr friend bool operator>=(tagged_id const &lhs, O const &rhs) noexcept { return lhs >= tagged_id{rhs}; }
    template<typename O> [[nodiscard]] constexpr friend bool operator==(O const &lhs, tagged_id const &rhs) noexcept { return tagged_id{lhs} == rhs; }
    template<typename O> [[nodiscard]] constexpr friend bool operator!=(O const &lhs, tagged_id const &rhs) noexcept { return tagged_id{lhs} != rhs; }
    template<typename O> [[nodiscard]] constexpr friend bool operator<(O const &lhs, tagged_id const &rhs) noexcept { return tagged_id{lhs} < rhs; }
    template<typename O> [[nodiscard]] constexpr friend bool operator>(O const &lhs, tagged_id const &rhs) noexcept { return tagged_id{lhs} > rhs; }
    template<typename O> [[nodiscard]] constexpr friend bool operator<=(O const &lhs, tagged_id const &rhs) noexcept { return tagged_id{lhs} <= rhs; }
    template<typename O> [[nodiscard]] constexpr friend bool operator>=(O const &lhs, tagged_id const &rhs) noexcept { return tagged_id{lhs} >= rhs; }

    [[nodiscard]] friend std::string to_string(tagged_id const &rhs) noexcept {
        return fmt::format("{}:{}", std::type_index(typeid(rhs.TAG)).name(), rhs.value);
    }

    friend std::ostream &operator<<(std::ostream &lhs, tagged_id const &rhs) {
        return lhs << to_string(rhs);
    }
};

}

namespace std {

template<typename T, typename Tag, tt::ssize_t Max>
struct hash<tt::tagged_id<T,Tag,Max>> {
    [[nodiscard]] constexpr size_t operator() (tt::tagged_id<T,Tag,Max> const &rhs) const noexcept {
        return rhs.hash();
    }
};

}