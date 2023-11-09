// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <string_view>
#include <string>
#include <span>
#include <bit>

export module hikogui_security_sip_hash;
import hikogui_random;
import hikogui_utility;

export namespace hi::inline v1 {
namespace detail {

struct sip_hash_seed_type {
    uint64_t k0;
    uint64_t k1;

    sip_hash_seed_type(uint64_t k0, uint64_t k1) noexcept : k0(k0), k1(k1) {}

    sip_hash_seed_type() noexcept : sip_hash_seed_type(seed<uint64_t>{}(), seed<uint64_t>{}()) {}
};

auto sip_hash_seed = sip_hash_seed_type();

struct sip_hash_seed_tag {};

} // namespace detail

template<size_t C, size_t D>
class sip_hash {
public:
    constexpr sip_hash(sip_hash const&) noexcept = default;
    constexpr sip_hash(sip_hash&&) noexcept = default;
    constexpr sip_hash& operator=(sip_hash const&) noexcept = default;
    constexpr sip_hash& operator=(sip_hash&&) noexcept = default;

    sip_hash(detail::sip_hash_seed_tag) noexcept : sip_hash(detail::sip_hash_seed.k0, detail::sip_hash_seed.k1) {}

    /** Create a sip_hash initialized with the global initialized key.
     */
    sip_hash() noexcept;

    constexpr sip_hash(uint64_t k0, uint64_t k1) noexcept :
        _v0(k0 ^ 0x736f6d6570736575),
        _v1(k1 ^ 0x646f72616e646f6d),
        _v2(k0 ^ 0x6c7967656e657261),
        _v3(k1 ^ 0x7465646279746573),
        _m(0),
        _b(0)
    {
#ifndef NDEBUG
        _debug_state = debug_state_type::idle;
#endif
    }

    [[nodiscard]] uint64_t finish() noexcept
    {
#ifndef NDEBUG
        hi_assert(_debug_state < debug_state_type::finalized);
        _debug_state = debug_state_type::finalized;
#endif

        auto v0 = _v0;
        auto v1 = _v1;
        auto v2 = _v2;
        auto v3 = _v3;
        auto m = _m;
        auto b = _b;

        // Add the length modulo 256 to the end of the last block.
        m |= static_cast<uint64_t>(b) << 56;
        _compress(v0, v1, v2, v3, m);
        _finalize(v0, v1, v2, v3);

        return v0 ^ v1 ^ v2 ^ v3;
    }

    void add(void const *data, size_t size) noexcept
    {
#ifndef NDEBUG
        hi_assert(_debug_state <= debug_state_type::partial);
        _debug_state = debug_state_type::partial;
#endif
        auto todo = size;
        auto *src = static_cast<char const *>(data);
        hi_axiom_not_null(src);

        auto v0 = _v0;
        auto v1 = _v1;
        auto v2 = _v2;
        auto v3 = _v3;
        auto m = _m;

        // If a partial 64-bit word was already submitted, complete that word.
        if (hilet offset = _b & 7) {
            hilet num_bytes = std::min(8_uz - offset, size);

            // Accumulate remaining bytes in m.
            for (auto i = offset; i != offset + num_bytes; ++i) {
                m |= char_cast<uint64_t>(src[i]) << (i * CHAR_BIT);
            }

            if (offset + num_bytes == 8) {
                _compress(v0, v1, v2, v3, std::exchange(m, 0));
            }

            todo -= num_bytes;
            src += num_bytes;
        }

        // Now we can compress 64 bits at a time.
        while (todo >= 8) {
            m = load_le<uint64_t>(src);
            src += 8;
            todo -= 8;
            _compress(v0, v1, v2, v3, std::exchange(m, 0));
        }

        // Add the incomplete word in the state, to be compressed later.
        for (auto i = 0_uz; i != todo; ++i) {
            m |= char_cast<uint64_t>(src[i]) << (i * CHAR_BIT);
        }

        _v0 = v0;
        _v1 = v1;
        _v2 = v2;
        _v3 = v3;
        _m = m;
        _b = truncate<uint8_t>(_b + size);
    }

    /** Hash a complete message.
     *
     * This function is significantly faster than using `add()` and `finish()`.
     *
     * @param data The data to hash.
     * @param size The size of the data in bytes.
     * @return The value of the hash.
     * @note The `sip_hash` instance can be reused when using this function
     */
    [[nodiscard]] uint64_t complete_message(void const *data, size_t size) const noexcept
    {
        auto *src = static_cast<char const *>(data);
        hi_axiom_not_null(src);

#ifndef NDEBUG
        hi_assert(_debug_state == debug_state_type::idle);
#endif

        auto v0 = _v0;
        auto v1 = _v1;
        auto v2 = _v2;
        auto v3 = _v3;
        uint64_t m;

        for (auto block_count = size / 8; block_count > 0; --block_count, src += 8) {
            m = load_le<uint64_t>(src);
            _compress(v0, v1, v2, v3, m);
        }

        // The length, and 0 to 7 of the last bytes from the src.
        m = wide_cast<uint64_t>(size & 0xff) << 56;

        for (auto i = 0_uz; i != (size & 7); ++i) {
            m |= char_cast<uint64_t>(src[i]) << (i * CHAR_BIT);
        }
        _compress(v0, v1, v2, v3, m);
        _finalize(v0, v1, v2, v3);

        return v0 ^ v1 ^ v2 ^ v3;
    }

    /** Hash a complete message.
     *
     * @see complete_message()
     */
    [[nodiscard]] uint64_t operator()(void const *data, size_t size) const noexcept
    {
        return complete_message(data, size);
    }

private:
    uint64_t _v0;
    uint64_t _v1;
    uint64_t _v2;
    uint64_t _v3;

    uint64_t _m;
    uint8_t _b;
#ifndef NDEBUG
    enum class debug_state_type : uint8_t { idle, full, partial, finalized };
    debug_state_type _debug_state;
#endif

    hi_force_inline constexpr static void _round(uint64_t& v0, uint64_t& v1, uint64_t& v2, uint64_t& v3) noexcept
    {
        v0 += v1;
        v2 += v3;
        v1 = std::rotl(v1, 13);
        v3 = std::rotl(v3, 16);
        v1 ^= v0;
        v3 ^= v2;
        v0 = std::rotl(v0, 32);

        v0 += v3;
        v2 += v1;
        v1 = std::rotl(v1, 17);
        v3 = std::rotl(v3, 21);
        v1 ^= v2;
        v3 ^= v0;
        v2 = std::rotl(v2, 32);
    }

    constexpr static void _compress(uint64_t& v0, uint64_t& v1, uint64_t& v2, uint64_t& v3, uint64_t m) noexcept
    {
        hilet m_ = m;

        v3 ^= m_;
        for (auto i = 0_uz; i != C; ++i) {
            _round(v0, v1, v2, v3);
        }
        v0 ^= m_;
    }

    constexpr static void _finalize(uint64_t& v0, uint64_t& v1, uint64_t& v2, uint64_t& v3) noexcept
    {
        v2 ^= 0xff;
        for (auto i = 0_uz; i != D; ++i) {
            _round(v0, v1, v2, v3);
        }
    }
};

namespace detail {
template<size_t C, size_t D>
sip_hash sip_hash_prototype = sip_hash<C, D>(sip_hash_seed_tag{});
}

template<size_t C, size_t D>
sip_hash<C, D>::sip_hash() noexcept : sip_hash(detail::sip_hash_prototype<C, D>)
{
}

using _sip_hash24 = sip_hash<2, 4>;

template<typename T>
struct sip_hash24 {
    [[nodiscard]] uint64_t operator()(T const& rhs) const noexcept
    {
        hi_static_not_implemented();
    }

    [[nodiscard]] uint64_t operator()(T const& rhs) const noexcept
        requires(std::has_unique_object_representations_v<T> and not std::is_pointer_v<T>)
    {
        return _sip_hash24{}(&rhs, sizeof(rhs));
    }
};

template<typename CharT, typename CharTrait>
struct sip_hash24<std::basic_string_view<CharT, CharTrait>> {
    [[nodiscard]] uint64_t operator()(std::basic_string_view<CharT, CharTrait> const& rhs) const noexcept
    {
        return _sip_hash24{}(rhs.data(), rhs.size());
    }
};

template<typename CharT, typename CharTrait>
struct sip_hash24<std::basic_string<CharT, CharTrait>> {
    [[nodiscard]] uint64_t operator()(std::basic_string<CharT, CharTrait> const& rhs) const noexcept
    {
        return _sip_hash24{}(rhs.data(), rhs.size());
    }
};

template<typename T>
struct sip_hash24<std::span<T>> {
    [[nodiscard]] uint64_t operator()(std::span<T> const& rhs) const noexcept
    {
        return _sip_hash24{}(rhs.data(), rhs.size());
    }
};

template<>
struct sip_hash24<char *> {
    [[nodiscard]] uint64_t operator()(char const *rhs) const noexcept
    {
        hilet length = strlen(rhs);
        return _sip_hash24{}(rhs, length * sizeof(char));
    }
};

template<>
struct sip_hash24<wchar_t *> {
    [[nodiscard]] uint64_t operator()(wchar_t const *rhs) const noexcept
    {
        hilet length = wcslen(rhs);
        return _sip_hash24{}(rhs, length * sizeof(wchar_t));
    }
};

} // namespace hi::inline v1
