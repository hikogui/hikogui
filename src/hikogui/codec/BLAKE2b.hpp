

namespace hi::inline v1 {

class BLAKE2b {
public:

private:
    // Sigma is pre-shifted by 3 as-if to multiply by sizeof(uint64_t)
    constexpr static auto _sigma = std::array<uint64,16>{
        0xfedcba9876543210ULL,
        0x357b20c16df984aeULL,
        0x491763eadf250c8bULL,
        0x8f04a562ebcd1397ULL,
        0xd386cb1efa427509ULL,
        0x91ef57d438b0a6c2ULL,
        0xb8293670a4def15cULL,
        0xa2684f05931ce7bdULL,
        0x5a417d2c803b9ef6ULL,
        0x0dc3e9bf5167482aULL,
        0xfedcba9876543210ULL, // First two are repeated for 12 rounds
        0x357b20c16df984aeULL};

    hi_force_inline constexpr static u64x2 _load(uint64_t *chunk, uint64_t &s) noexcept
    {
        hilet v0 = u64x2::load1(chunk[s & 0xf]);
        s >>= 4;
        hilet v1 = u64x2::load1(chunk[s & 0xf]);
        s >>= 4;
        return v0 | v1.yx();
    }

    hi_force_inline constexpr static void _round(
        u64x2 &v0_10,
        u64x2 &v1_11,
        u64x2 &v2_8,
        u64x2 &v3_9,
        u64x2 &v4_14,
        u64x2 &v5_15,
        u64x2 &v6_12,
        u64x2 &v7_13,
        uint64_t *chunk,
        size_t i)
    {
        auto s = _sigma[i];

        auto v0_8 = merge<0, 1>(v0_10, v2_8);
        auto v4_12 = merge<0, 1>(v4_14, v6_12);
        _mix(v0_8, v4_12, _load(chunk, s));

        auto v1_9 = merge<0, 1>(v1_11, v3_9);
        auto v5_13 = merge<0, 1>(v5_15, v7_13);
        _mix(v1_9, v5_13, _load(chunk, s));

        auto v2_10 = merge<0, 1>(v2_8, v0_10);
        auto v6_14 = merge<0, 1>(v6_12, v4_14);
        _mix(v2_10, v6_14, _load(chunk, s));

        auto v3_11 = merge<0, 1>(v3_9, v1_11);
        auto v7_15 = merge<0, 1>(v7_13, v5_15);
        _mix(v3_11, v7_15, _load(chunk, s));


        v0_10 = merge<0, 1>(v0_8, v2_10);
        v5_15 = merge<0, 1>(v5_13, v7_15);
        _mix(v0_10, v5_15, _load(chunk, s));

        v1_11 = merge<0, 1>(v1_9, v3_11);
        v6_12 = merge<0, 1>(v6_14, v4_12);
        _mix(v1_11, v6_12, _load(chunk, s));

        v2_8 = merge<0, 1>(v2_10, v0_8);
        v7_13 = merge<0, 1>(v7_15, v5_13);
        _mix(v2_8, v7_13, _load(chunk, s));

        v3_9 = merge<0, 1>(v3_11, v1_9);
        v4_14 = merge<0, 1>(v4_12, v6_14);
        _mix(v3_9, v4_14, _load(chunk, s));
    }

    hi_force_inline constexpr static void _mix(u64x2 &ac, u64x2 &bd, u64x2 m)
    {
        ac = ac + bd + m.x0();
        bd = rotr<24, 32>(bd ^ ac.yx());

        ac = ac + bd + m.y0();
        bd = rotr<63, 16>(bd ^ ac.yx());
    }
};

}

