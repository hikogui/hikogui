
#include "sha2.hpp"

namespace tt {

template<typename T>
constexpr T Ch(T x, T y, T z) noexcept {
    return (x & y) ^ (~x & z);
}

template<typename T>
constexpr T Maj(T x, T y, T z) noexcept {
    return (x & y) ^ (x & z) ^ (y & z);
}

template<typename T, int N>
constexpr T rotr(T x) noexcept {
    return x >> N | x << (sizeof(T)*CHAR_BIT - N);
}

template<typename T, int A, int B, int C>
constexpr T S(T x) noexcept {
    return rotr<A>(x) ^ rotr<B>(x) ^ rotr<C>(x);
}

template<typename T, int A, int B, int C>
constexpr T s(T x) noexcept {
    return rotr<A>(x) ^ rotr<B>(x) ^ x >> C;
}

template<typename T> constexpr T S0(T x) noexcept;
template<> constexpr uint32_t S0(uint32_t x) { return S<2,13,22>(x); }
template<> constexpr uint64_t S0(uint64_t x) { return S<28,34,39>(x); }

template<typename T> constexpr T S1(T x) noexcept;
template<> constexpr uint32_t S1(uint32_t x) { return S<6,11,25>(x); }
template<> constexpr uint64_t S1(uint64_t x) { return S<14,18,41>(x); }

template<typename T> constexpr T s0(T x) noexcept;
template<> constexpr uint32_t s0(uint32_t x) { return s<7,18,3>(x); }
template<> constexpr uint64_t s0(uint64_t x) { return s<1,61,6>(x); }

template<typename T> constexpr T s1(T x) noexcept;
template<> constexpr uint32_t s1(uint32_t x) { return s<17,19,10>(x); }
template<> constexpr uint64_t s1(uint64_t x) { return s<19,61,6>(x); }

template<typename T>
constexpr std::array<T,80> K;

template<>
constexpr std::array<uint32_t,80> K = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

template<>
constexpr std::array<uint64_t,80> K = {
    0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc, 0x3956c25bf348b538, 
    0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118, 0xd807aa98a3030242, 0x12835b0145706fbe, 
    0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2, 0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 
    0xc19bf174cf692694, 0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65, 
    0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5, 0x983e5152ee66dfab, 
    0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4, 0xc6e00bf33da88fc2, 0xd5a79147930aa725, 
    0x06ca6351e003826f, 0x142929670a0e6e70, 0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 
    0x53380d139d95b3df, 0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b, 
    0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30, 0xd192e819d6ef5218, 
    0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8, 0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 
    0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8, 0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 
    0x682e6ff3d6b2b8a3, 0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec, 
    0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b, 0xca273eceea26619c, 
    0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178, 0x06f067aa72176fba, 0x0a637dc5a2c898a6, 
    0x113f9804bef90dae, 0x1b710b35131c471b, 0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 
    0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

template<int N>
[[nodiscard]] std::array<std::byte,N> sha2_output(sha2::state_type const &state) noexcept
{
    std::array<std::byte,N> r;

    for (int i = 0; i != N; ++i) {
        r[i] = state[i / 8] >> (56 - (i % 8) * 8);
    }

    return r;
}

template<typename T>
constexpr void sha2_round(std::array<T,8> &state, T K, T W) noexcept
{
    ttlet T1 =
        state[7] +
        S1(state[4]) +
        Ch(state[4], state[5], state[6]) +
        K +
        W;

    ttlet T2 =
        S0(state[0]) +
        Maj(state[0], state[1], state[2]);

    state[7] = state[6];
    state[6] = state[5];
    state[5] = state[4];
    state[4] = state[3] + T1;
    state[3] = state[2];
    state[2] = state[1];
    state[1] = state[0];
    state[0] = T1 + T2;
}

template<typename T>
constexpr void sha2_block(std::array<T,8> &state, std::array<T,16> const &block) noexcept
{
    std::array<T,16> W;

    ttlet tmp_state = state;
    for (auto j = 0; j != 16; ++j) {
        sha2_round(
            tmp_state,
            K[j],
            W[j] = block[j]
        );
    }
    for (auto j = 16; j != 80; ++j) {
        sha2_round(
            tmp_state,
            K<T>[j], 
            W[j & 0xf] =
                s1(W[(j- 2) & 0xf]) +
                   W[(j- 7) & 0xf] +
                s0(W[(j-15) & 0xf]) +
                   W[(j-16) & 0xf]
        );
    }
    state += tmp_state;
}

template<typename N>
std::array<std::byte,N> sha2(byte_string data)
{
    sha2::state_type state;
    if constexpr (N == 64) {
        state[0] = 0x6a09e667f3bcc908;
        state[1] = 0xbb67ae8584caa73b;
        state[2] = 0x3c6ef372fe94f82b;
        state[3] = 0xa54ff53a5f1d36f1;
        state[4] = 0x510e527fade682d1;
        state[5] = 0x9b05688c2b3e6c1f;
        state[6] = 0x1f83d9abfb41bd6b;
        state[7] = 0x5be0cd19137e2179;
    } else if constexpr (N == 32) {
        state[0] = 0x22312194FC2BF72C;
        state[1] = 0x9F555FA3C84C64C2;
        state[2] = 0x2393B86B6F53B151;
        state[3] = 0x963877195940EABD;
        state[4] = 0x96283EE2A88EFFE3;
        state[5] = 0xBE5E1E2553863992;
        state[6] = 0x2B0199FC2C85B8AA;
        state[7] = 0x0EB72DDC81C52CA2;
    } else {
        tt_no_default;
    }

    return sha2_output<N>(state);
}

}
