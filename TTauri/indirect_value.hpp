// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <memory>

namespace TTauri {

template<typename T>
class indirect_value {
    T *ptr;

public:
    ~indirect_value() {
        if (ptr) {
            delete ptr;
        }
    }

    indirect_value(indirect_value const &other) {
        ptr = new T;
        *ptr = *(other.ptr);
    }

    indirect_value(indirect_value &&other) {
        ptr = other.ptr;
        other.ptr = nullptr;
    }

    indirect_value &operator=(T const &value) {
        *ptr = value;
        return *this;
    }

    indirect_value &operator=(T &&value) {
        *ptr = std::move(value);
        return *this;
    }

    indirect_value(T const &value) {
        ptr = new T;
        *ptr = value;
    }

    indirect_value(T &&value) {
        ptr = new T;
        *ptr = std::move(value);
    }

    indirect_value &operator=(indirect_value const &other) {
        *ptr = *(other.ptr);
        return *this;
    }

    indirect_value &operator=(indirect_value &&other) {
        delete ptr;
        ptr = other.ptr;
        other.ptr = nullptr;
        return *this;
    }

    operator T() const {
        return *ptr;
    }


    // Operators.
    T const *operator->() const { return ptr; }
    T *operator->() { return ptr; }
    T const &operator*() const { return *ptr; }
    T &operator*() { return *ptr; }

    template<typename U> indirect_value &operator+=(U const &other) { *ptr += other; return *this; }
    template<typename U> indirect_value &operator-=(U const &other) { *ptr -= other; return *this; }
    template<typename U> indirect_value &operator*=(U const &other) { *ptr *= other; return *this; }
    template<typename U> indirect_value &operator/=(U const &other) { *ptr /= other; return *this; }
    template<typename U> indirect_value &operator%=(U const &other) { *ptr %= other; return *this; }
    template<typename U> indirect_value &operator<<=(U const &other) { *ptr <<= other; return *this; }
    template<typename U> indirect_value &operator>>=(U const &other) { *ptr >>= other; return *this; }
    template<typename U> indirect_value &operator^=(U const &other) { *ptr >>= other; return *this; }

    template<typename U> auto &operator[](U const &other) { return (*ptr)[other]; }
    template<typename U> auto const &operator[](U const &other) const { return (*ptr)[other]; }
};

template<typename T, typename U> auto operator+(indirect_value<T> const &lhs, U const &rhs) { return static_cast<T>(lhs) + rhs; }
template<typename T, typename U> auto operator+(T const &lhs, indirect_value<U> const &rhs) { return lhs + static_cast<T>(lhs); }
template<typename T, typename U> auto operator-(indirect_value<T> const &lhs, U const &rhs) { return static_cast<T>(lhs) - rhs; }
template<typename T, typename U> auto operator-(T const &lhs, indirect_value<U> const &rhs) { return lhs - static_cast<T>(lhs); }
template<typename T, typename U> auto operator*(indirect_value<T> const &lhs, U const &rhs) { return static_cast<T>(lhs) * rhs; }
template<typename T, typename U> auto operator*(T const &lhs, indirect_value<U> const &rhs) { return lhs * static_cast<T>(lhs); }
template<typename T, typename U> auto operator/(indirect_value<T> const &lhs, U const &rhs) { return static_cast<T>(lhs) / rhs; }
template<typename T, typename U> auto operator/(T const &lhs, indirect_value<U> const &rhs) { return lhs / static_cast<T>(lhs); }
template<typename T, typename U> auto operator%(indirect_value<T> const &lhs, U const &rhs) { return static_cast<T>(lhs) % rhs; }
template<typename T, typename U> auto operator%(T const &lhs, indirect_value<U> const &rhs) { return lhs % static_cast<T>(lhs); }
template<typename T, typename U> auto operator<<(indirect_value<T> const &lhs, U const &rhs) { return static_cast<T>(lhs) << rhs; }
template<typename T, typename U> auto operator<<(T const &lhs, indirect_value<U> const &rhs) { return lhs << static_cast<T>(lhs); }
template<typename T, typename U> auto operator>>(indirect_value<T> const &lhs, U const &rhs) { return static_cast<T>(lhs) >> rhs; }
template<typename T, typename U> auto operator>>(T const &lhs, indirect_value<U> const &rhs) { return lhs >> static_cast<T>(lhs); }
template<typename T, typename U> auto operator^(indirect_value<T> const &lhs, U const &rhs) { return static_cast<T>(lhs) ^ rhs; }
template<typename T, typename U> auto operator^(T const &lhs, indirect_value<U> const &rhs) { return lhs ^ static_cast<T>(lhs); }



}
