// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file random/seed.hpp - Cryptographically secure entropy.
 */

module;
#include "../macros.hpp"

#include <random>
#include <concepts>
#include <type_traits>
#include <bit>
#include <array>

export module hikogui_random_seed : intf;
import hikogui_SIMD;
import hikogui_utility;

export namespace hi::inline v1 {

/** Load a random seed.
 *
 * @param[out] ptr The pointer to the buffer bytes where to generate the seed.
 * @param size The number of bytes to generate.
 * @throws os_error on failure.
 */
void generate_seed(void *ptr, size_t size);

/** Randomly generate an object.
 *
 * Each specialization defines the `operator() const` that will return randomly
 * generated instance of the type.
 *
 * @tparam T The type to return from `operator() const`.
 */
template<typename T>
struct seed {
    [[nodiscard]] T operator()() const
    {
        hi_not_implemented();
    }

    [[nodiscard]] T operator()() const requires(std::has_unique_object_representations_v<T> and not std::is_pointer_v<T>)
    {
        auto buffer = std::array<uint8_t, sizeof(T)>{};
        generate_seed(buffer.data(), buffer.size());
        return std::bit_cast<T>(buffer);
    }
};

} // namespace hi::inline v1
