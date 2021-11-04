// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../assert.hpp"
#include "../rapid/numeric_array.hpp"
#include <random>
#include <concepts>

namespace tt::inline v1 {

class seed_generator {
public:
    seed_generator(seed_generator const &) noexcept = default;
    seed_generator(seed_generator &&) noexcept = default;
    seed_generator &operator=(seed_generator const &) noexcept = default;
    seed_generator &operator=(seed_generator &&) noexcept = default;

    [[nodiscard]] seed_generator() noexcept : _device(std::random_device{}) {}

    [[nodiscard]] seed_generator(std::string const &name) noexcept : _device(std::random_device{name}) {}

    template<typename T>
    [[nodiscard]] T next() noexcept
    {
        tt_no_default();
    }

    template<>
    [[nodiscard]] unsigned int next() noexcept
    {
        return _device();
    }

    template<>
    [[nodiscard]] unsigned long next() noexcept
    {
        if constexpr (sizeof(unsigned int) == sizeof(unsigned long)) {
            return static_cast<unsigned long>(next<unsigned int>());
        } else {
            auto r = static_cast<unsigned long>(next<unsigned int>());
            r <<= sizeof(unsigned int) * CHAR_BIT;
            r |= static_cast<unsigned long>(next<unsigned int>());
            return r;
        }
    }

    template<>
    [[nodiscard]] unsigned long long next() noexcept
    {
        if constexpr (sizeof(unsigned long) == sizeof(unsigned long long)) {
            return static_cast<unsigned long long>(next<unsigned long>());
        } else {
            auto r = static_cast<unsigned long long>(next<unsigned long>());
            r <<= sizeof(unsigned long) * CHAR_BIT;
            r |= static_cast<unsigned long long>(next<unsigned long>());
            return r;
        }
    }

    template<>
    [[nodiscard]] unsigned short next() noexcept
    {
        return static_cast<unsigned short>(next<unsigned int>());
    }

    template<>
    [[nodiscard]] unsigned char next() noexcept
    {
        return static_cast<unsigned char>(next<unsigned int>());
    }

    template<>
    [[nodiscard]] u64x2 next() noexcept
    {
        return u64x2{next<uint64_t>(), next<uint64_t>()};
    }

    template<typename T>
    [[nodiscard]] T next_not_zero() noexcept
    {
        T r;

        do {
            r = next<T>();
        } while (r == 0);

        return r;
    }

    template<>
    [[nodiscard]] u64x2 next_not_zero() noexcept
    {
        return u64x2{next_not_zero<uint64_t>(), next_not_zero<uint64_t>()};
    }

private:
    std::random_device _device;
};

} // namespace tt::inline v1
