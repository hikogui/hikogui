// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"
#include "TTauri/Foundation/assert.hpp"
#include <array>
#include <memory>
#include <type_traits>

namespace TTauri {

template<typename T>
constexpr bool should_call_destructor() {
    return std::is_destructible_v<T> && (!std::is_trivially_destructible_v<T> || std::has_virtual_destructor_v<T>);
}

template<typename T, size_t S>
class polymorphic_value {
private:
    using base_type = T;
    static constexpr size_t capacity = S;

    alignas(16) std::array<std::byte,capacity> _data;
    bool has_value;

public:
    polymorphic_value() : has_value(false) {}

    ~polymorphic_value() {
        if constexpr (should_call_destructor<T>()) {
            if (has_value) {
                std::destroy_at(this->operator->());
            }
        }
    }

    polymorphic_value(polymorphic_value const &other) = delete;
    polymorphic_value(polymorphic_value &&other) = delete;
    polymorphic_value &operator=(polymorphic_value const &other) = delete;
    polymorphic_value &operator=(polymorphic_value &&other) = delete;

    template<typename O>
    std::enable_if_t<std::is_base_of_v<base_type, O>, polymorphic_value> &operator=(O const &other) {
        static_assert(sizeof(O) <= capacity, "Assignment of a type larger than capacity of polymorphic_value");
        reset();
        new(data()) O(other);

        if constexpr (should_call_destructor<T>()) {
            has_value = true;
        }
        return *this;
    }

    template<typename O>
    std::enable_if_t<std::is_base_of_v<base_type, O>, polymorphic_value> &operator=(O &&other) {
        static_assert(sizeof(O) <= capacity, "Assignment of a type larger than capacity of polymorphic_value");
        reset();
        new(data()) O(std::forward<O>(other));

        if constexpr (should_call_destructor<T>()) {
            has_value = true;
        }
        return *this;
    }

    template<typename O, typename... Args>
    std::enable_if_t<std::is_base_of_v<base_type, O>, void> emplace(Args &&... args) {
        static_assert(sizeof(O) <= capacity, "Assignment of a type larger than capacity of polymorphic_value");
        reset();
        new(data()) O(std::forward<Args>(args)...);

        if constexpr (should_call_destructor<T>()) {
            has_value = true;
        }
    }

    void reset() noexcept {
        if constexpr (should_call_destructor<T>()) {
            if (has_value) {
                std::destroy_at(this->operator->());
            }
            has_value = false;
        }
    }

    void *data() noexcept {
        return reinterpret_cast<void *>(_data.data());
    }

    void const *data() const noexcept {
        return reinterpret_cast<void const *>(_data.data());
    }

    T const &operator*() const noexcept {
        if constexpr (should_call_destructor<T>()) {
            required_assert(has_value);
        }
        return *reinterpret_cast<T const *>(data());
    }

    T &operator*() noexcept {
        if constexpr (should_call_destructor<T>()) {
            required_assert(has_value);
        }
        return *reinterpret_cast<T *>(data());
    }

    T const *operator->() const noexcept {
        if constexpr (should_call_destructor<T>()) {
            required_assert(has_value);
        }
        return reinterpret_cast<T const *>(data());
    }

    T *operator->() noexcept {
        if constexpr (should_call_destructor<T>()) {
            required_assert(has_value);
        }
        return reinterpret_cast<T *>(data());
    }
};

}