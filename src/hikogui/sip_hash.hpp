
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

struct sip_hash_seed_tag {};

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
    }

    [[nodiscard]] value_type finish() noexcept
    {
        auto tmp_m = (_b & 7) == 0 ? 0 : _m;

        // Add the length modulo 256 to the end of the last block.
        tmp_m |= static_cast<uint64_t>(_b) << 56;
        _compress(tmp_m);
        return _finalize();
    }

    void add(void const *data, size_t size) noexcept
    {
        auto todo = size;
        auto tmp_m = _m;

        auto *src = reinterpret_cast<char const *>(data);

        // If a partial 64-bit word was already submitted, complete that word.
        if (hilet offset = _b & 7) {
            hilet num_bytes = std::min(8_uz - offset, size);
            unaligned_load_le(tmp_m, src, num_bytes, offset);

            if (offset + num_bytes == 8) {
                _compress(std::exchange(tmp_m, 0));
            }

            todo -= num_bytes;
            src += num_bytes;
        }

        // Now we can compress 64 bits at a time.
        while (todo >= 8) {
            unaligned_load_le(tmp_m, src);
            src += 8;
            todo -= 8;
            _compress(std::exchange(tmp_m, 0));
        }

        // Add the incomplete word in the state, to be compressed later.
        if (todo) {
            unaligned_load_le(tmp_m, src, todo);
        }

        _m = tmp_m;
        _b = static_cast<uint8_t>(_b + size);
    }

private:
    value_type _v0;
    value_type _v1;
    value_type _v2;
    value_type _v3;

    uint64_t _m;
    uint8_t _b;

    static constexpr void _round(value_type& v0, value_type& v1, value_type& v2, value_type& v3) noexcept
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

    constexpr void _compress(uint64_t m) noexcept
    {
        hilet m_ = broadcast<value_type>{}(m);

        auto v0 = _v0;
        auto v1 = _v1;
        auto v2 = _v2;
        auto v3 = _v3;

        v3 ^= m_;
        for (auto i = 0_uz; i != C; ++i) {
            _round(v0, v1, v2, v3);
        }
        v0 ^= m_;

        _v0 = v0;
        _v1 = v1;
        _v2 = v2;
        _v3 = v3;
    }

    [[nodiscard]] constexpr value_type _finalize() const noexcept
    {
        auto v0 = _v0;
        auto v1 = _v1;
        auto v2 = _v2;
        auto v3 = _v3;

        v2 ^= broadcast<value_type>{}(0xff);
        for (auto i = 0_uz; i != D; ++i) {
            _round(v0, v1, v2, v3);
        }

        return v0 ^ v1 ^ v2 ^ v3;
    }
};

namespace detail {
template<typename T, size_t C, size_t D>
static inline sip_hash sip_hash_prototype = sip_hash<T, C, D>(sip_hash_seed_tag{});
}

template<typename T, size_t C, size_t D>
sip_hash<T,C,D>::sip_hash() noexcept : sip_hash(detail::sip_hash_prototype<T,C,D>) {}


using sip_hash24 = sip_hash<uint64_t, 2, 4>;
using sip_hash24x2 = sip_hash<u64x2, 2, 4>;
using sip_hash24x4 = sip_hash<u64x4, 2, 4>;

} // namespace hi::inline v1
