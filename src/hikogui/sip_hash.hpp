
#include "rapid/numeric_array.hpp"
#include "random/seed.hpp"
#include "math.hpp"
#include "endian.hpp"

namespace hi::inline v1 {
namespace detail {

struct sip_hash_seed_type {
    u64x4 k0_x4;
    u64x4 k1_x4;
    u64x2 k0_x2;
    u64x2 k1_x2;
    uint64_t k0_x1;
    uint64_t k1_x1;

    sip_hash_seed_type(u64x4 k0, u64x4 k1) noexcept :
        k0_x4(k0), k1_x4(k1), k0_x2{k0.x(), k0.y()}, k1_x2{k1.x(), k1.y()}, k0_x1{k0.x()}, k1_x1{k1.x()}
    {
    }

    sip_hash_seed_type() noexcept : sip_hash_seed_type(seed<u64x4>{}(), seed<u64x4>{}()) {}
};

inline auto sip_hash_seed = sip_hash_seed_type();

struct sip_hash_seed_tag {
};

} // namespace detail

template<typename T, size_t C, size_t D>
class sip_hash {
public:
    using value_type = T;

    constexpr sip_hash(sip_hash const&) noexcept = default;
    constexpr sip_hash(sip_hash&&) noexcept = default;
    constexpr sip_hash& operator=(sip_hash const&) noexcept = default;
    constexpr sip_hash& operator=(sip_hash&&) noexcept = default;

    sip_hash(detail::sip_hash_seed_tag) noexcept requires(std::is_same_v<value_type, uint64_t>) :
        sip_hash(detail::sip_hash_seed.k0_x1, detail::sip_hash_seed.k1_x1)
    {
    }

    sip_hash(detail::sip_hash_seed_tag) noexcept requires(std::is_same_v<value_type, u64x2>) :
        sip_hash(detail::sip_hash_seed.k0_x2, detail::sip_hash_seed.k1_x2)
    {
    }

    sip_hash(detail::sip_hash_seed_tag) noexcept requires(std::is_same_v<value_type, u64x4>) :
        sip_hash(detail::sip_hash_seed.k0_x4, detail::sip_hash_seed.k1_x4)
    {
    }

    /** Create a sip_hash initialized with the global initialized key.
     */
    sip_hash() noexcept;

    constexpr sip_hash(value_type k0, value_type k1) noexcept :
        _v0(k0 ^ broadcast<value_type>{}(0x736f6d6570736575)),
        _v1(k1 ^ broadcast<value_type>{}(0x646f72616e646f6d)),
        _v2(k0 ^ broadcast<value_type>{}(0x6c7967656e657261)),
        _v3(k1 ^ broadcast<value_type>{}(0x7465646279746573)),
        _m(0),
        _b(0)
    {
#if HI_BUILT_TYPE == HI_BT_DEBUG
        _debug_state = debug_state_type::idle;
#endif
    }

    [[nodiscard]] value_type finish() noexcept
    {
#if HI_BUILT_TYPE == HI_BT_DEBUG
        hi_axiom(_debug_state < debug_state_type::finalized);
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
#if HI_BUILT_TYPE == HI_BT_DEBUG
        hi_axiom(_debug_state <= debug_state_type::partial);
        _debug_state = debug_state_type::partial;
#endif
        auto todo = size;
        auto *src = reinterpret_cast<char const *>(data);

        auto v0 = _v0;
        auto v1 = _v1;
        auto v2 = _v2;
        auto v3 = _v3;
        auto m = _m;

        // If a partial 64-bit word was already submitted, complete that word.
        if (hilet offset = _b & 7) {
            hilet num_bytes = std::min(8_uz - offset, size);
            unaligned_load_le(m, src, num_bytes, offset);

            if (offset + num_bytes == 8) {
                _compress(v0, v1, v2, v3, std::exchange(m, 0));
            }

            todo -= num_bytes;
            src += num_bytes;
        }

        // Now we can compress 64 bits at a time.
        while (todo >= 8) {
            unaligned_load_le(m, src);
            src += 8;
            todo -= 8;
            _compress(v0, v1, v2, v3, std::exchange(m, 0));
        }

        // Add the incomplete word in the state, to be compressed later.
        if (todo) {
            unaligned_load_le(m, src, todo);
        }

        _v0 = v0;
        _v1 = v1;
        _v2 = v2;
        _v3 = v3;
        _m = m;
        _b = static_cast<uint8_t>(_b + size);
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
    [[nodiscard]] value_type complete_message(void const *data, size_t size) const noexcept
    {
        auto *src = reinterpret_cast<char const *>(data);

#if HI_BUILT_TYPE == HI_BT_DEBUG
        hi_axiom(_debug_state == debug_state_type::idle);
#endif

        auto v0 = _v0;
        auto v1 = _v1;
        auto v2 = _v2;
        auto v3 = _v3;
        uint64_t m;

        for (auto block_count = size / 8; block_count > 0; --block_count, src += 8) {
            unaligned_load_le(m, src);
            _compress(v0, v1, v2, v3, m);
        }

        // The length, and 0 to 7 of the last bytes from the src.
        m = wide_cast<uint64_t>(size & 0xff) << 56;
        unaligned_load_le(m, src, size & 7);
        _compress(v0, v1, v2, v3, m);
        _finalize(v0, v1, v2, v3);

        return v0 ^ v1 ^ v2 ^ v3;
    }

    /** Hash a complete message.
     *
     * @see complete_message()
     */
    [[nodiscard]] value_type operator()(void const *data, size_t size) const noexcept
    {
        return complete_message(data, size);
    }

private:
    value_type _v0;
    value_type _v1;
    value_type _v2;
    value_type _v3;

    uint64_t _m;
    uint8_t _b;
#if HI_BUILT_TYPE == HI_BT_DEBUG
    enum class debug_state_type : uint8_t { idle, full, partial, finalized };
    state_type _debug_state;
#endif

    hi_force_inline static constexpr void _round(value_type& v0, value_type& v1, value_type& v2, value_type& v3) noexcept
    {
        using std::rotl;

        v0 += v1;
        v2 += v3;
        v1 = rotl(v1, 13);
        v3 = rotl(v3, 16);
        v1 ^= v0;
        v3 ^= v2;
        v0 = rotl(v0, 32);

        v0 += v3;
        v2 += v1;
        v1 = rotl(v1, 17);
        v3 = rotl(v3, 21);
        v1 ^= v2;
        v3 ^= v0;
        v2 = rotl(v2, 32);
    }

    static constexpr void _compress(value_type& v0, value_type& v1, value_type& v2, value_type& v3, uint64_t m) noexcept
    {
        hilet m_ = broadcast<value_type>{}(m);

        v3 ^= m_;
        for (auto i = 0_uz; i != C; ++i) {
            _round(v0, v1, v2, v3);
        }
        v0 ^= m_;
    }

    static constexpr void _finalize(value_type& v0, value_type& v1, value_type& v2, value_type& v3) noexcept
    {
        v2 ^= broadcast<value_type>{}(0xff);
        for (auto i = 0_uz; i != D; ++i) {
            _round(v0, v1, v2, v3);
        }
    }
};

namespace detail {
template<typename T, size_t C, size_t D>
static inline sip_hash sip_hash_prototype = sip_hash<T, C, D>(sip_hash_seed_tag{});
}

template<typename T, size_t C, size_t D>
sip_hash<T, C, D>::sip_hash() noexcept : sip_hash(detail::sip_hash_prototype<T, C, D>)
{
}

using _sip_hash24 = sip_hash<uint64_t, 2, 4>;
using _sip_hash24x2 = sip_hash<u64x2, 2, 4>;
using _sip_hash24x4 = sip_hash<u64x4, 2, 4>;

template<typename T>
struct sip_hash24 {
    [[nodiscard]] uint64_t operator()(T const& value) const noexcept
    {
        hi_static_not_implemented();
    }
};

template<typename T>
struct sip_hash24x2 {
    [[nodiscard]] u64x2 operator()(T const& value) const noexcept
    {
        hi_static_not_implemented();
    }
};

template<typename T>
struct sip_hash24x4 {
    [[nodiscard]] u64x4 operator()(T const& value) const noexcept
    {
        hi_static_not_implemented();
    }
};

template<typename T>
requires(std::has_unique_object_representations_v<T> and not std::is_pointer_v<T>) struct sip_hash24<T> {
    [[nodiscard]] uint64_t operator()(T const& value) const noexcept
    {
        return _sip_hash24{}(&value, sizeof(value));
    }
};

template<typename T>
requires(std::has_unique_object_representations_v<T> and not std::is_pointer_v<T>) struct sip_hash24x2<T> {
    [[nodiscard]] u64x2 operator()(T const& value) const noexcept
    {
        return _sip_hash24x2{}(&value, sizeof(value));
    }
};

template<typename T>
requires(std::has_unique_object_representations_v<T> and not std::is_pointer_v<T>) struct sip_hash24x4<T> {
    [[nodiscard]] u64x4 operator()(T const& value) const noexcept
    {
        return _sip_hash24x4{}(&value, sizeof(value));
    }
};

template<>
struct sip_hash24<wchar_t const *> {
    [[nodiscard]] uint64_t operator()(wchar_t const *str) const noexcept
    {
        hilet length = wcslen(str);
        return _sip_hash24{}(str, length * sizeof(wchar_t));
    }
};

template<>
struct sip_hash24x2<wchar_t const *> {
    [[nodiscard]] u64x2 operator()(wchar_t const *str) const noexcept
    {
        hilet length = wcslen(str);
        return _sip_hash24x2{}(str, length * sizeof(wchar_t));
    }
};

template<>
struct sip_hash24x4<wchar_t const *> {
    [[nodiscard]] u64x4 operator()(wchar_t const *str) const noexcept
    {
        hilet length = wcslen(str);
        return _sip_hash24x4{}(str, length * sizeof(wchar_t));
    }
};

template<>
struct sip_hash24<wchar_t *> {
    [[nodiscard]] uint64_t operator()(wchar_t const *str) const noexcept
    {
        hilet length = wcslen(str);
        return _sip_hash24{}(str, length * sizeof(wchar_t));
    }
};

template<>
struct sip_hash24x2<wchar_t *> {
    [[nodiscard]] u64x2 operator()(wchar_t const *str) const noexcept
    {
        hilet length = wcslen(str);
        return _sip_hash24x2{}(str, length * sizeof(wchar_t));
    }
};

template<>
struct sip_hash24x4<wchar_t *> {
    [[nodiscard]] u64x4 operator()(wchar_t const *str) const noexcept
    {
        hilet length = wcslen(str);
        return _sip_hash24x4{}(str, length * sizeof(wchar_t));
    }
};
} // namespace hi::inline v1
