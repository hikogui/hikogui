
#include "sha512.hpp"

namespace tt {

constexpr uint64_t Ch(uint64_t x, uint64_t y, uint64_t z) noexcept
{
    return (x & y) ^ (~x & z);
}

constexpr uint64_t Maj(uint64_t x, uint64_t y, uint64_t z) noexcept
{
    return (x & y) ^ (x & z) ^ (y & z);
}

template<int N>
constexpr uint64_t S(uint64_t x) noexcept
{
    return (x >> N) | (x << (64 - N));
}

template<int N>
constexpr uint64_t R(uint64_t x) noexcept
{
    return x >> N;
}

constexpr uint64_t E0(uint64_t x) noexcept
{
    return S<28>(x) ^ S<34>(x) ^ S<39>(x);
}

constexpr uint64_t E1(uint64_t x) noexcept
{
    return S<14>(x) ^ S<18>(x) ^ S<41>(x);
}

constexpr uint64_t o0(uint64_t x) noexcept
{
    return S<1>(x) ^ S<8>(x) ^ R<7>(x);
}

constexpr uint64_t o1(uint64_t x) noexcept
{
    return S<10>(x) ^ S<61>(x) ^ R<6>(x);
}

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
    0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a47581
};

[[nodiscard]] static uint64_t const &a(sha512::state_type const &state) noexcept { return state[0]; }
[[nodiscard]] static uint64_t const &b(sha512::state_type const &state) noexcept { return state[1]; }
[[nodiscard]] static uint64_t const &c(sha512::state_type const &state) noexcept { return state[2]; }
[[nodiscard]] static uint64_t const &d(sha512::state_type const &state) noexcept { return state[3]; }
[[nodiscard]] static uint64_t const &e(sha512::state_type const &state) noexcept { return state[4]; }
[[nodiscard]] static uint64_t const &f(sha512::state_type const &state) noexcept { return state[5]; }
[[nodiscard]] static uint64_t const &g(sha512::state_type const &state) noexcept { return state[6]; }
[[nodiscard]] static uint64_t const &h(sha512::state_type const &state) noexcept { return state[7]; }

[[nodiscard]] static uint64_t &a(sha512::state_type &state) noexcept { return state[0]; }
[[nodiscard]] static uint64_t &b(sha512::state_type &state) noexcept { return state[1]; }
[[nodiscard]] static uint64_t &c(sha512::state_type &state) noexcept { return state[2]; }
[[nodiscard]] static uint64_t &d(sha512::state_type &state) noexcept { return state[3]; }
[[nodiscard]] static uint64_t &e(sha512::state_type &state) noexcept { return state[4]; }
[[nodiscard]] static uint64_t &f(sha512::state_type &state) noexcept { return state[5]; }
[[nodiscard]] static uint64_t &g(sha512::state_type &state) noexcept { return state[6]; }
[[nodiscard]] static uint64_t &h(sha512::state_type &state) noexcept { return state[7]; }

template<int N>
[[nodiscard]] std::array<std::byte,N> sha512_output(sha512::state_type const &state) noexcept
{
    std::array<std::byte,N> r;

    for (int i = 0; i != N; ++i) {
        r[i] = state[i/8] >> (56 - (i%8)*8);
    }

    return r;
}

static void sha512_round(sha512::state_type &state, uint64_t K, uint64_t W) noexcept
{
    ttlet T1 =
        h(state) +
        E1(e(state)) +
        Ch(e(state), f(state), g(state)) +
        K +
        W;

    ttlet T2 =
        E0(a(state)) +
        Maj(a(state), b(state), c(state));

    h(state) = g(state);
    g(state) = f(state);
    f(state) = e(state);
    e(state) = d(state) + T1;
    d(state) = c(state);
    c(state) = b(state);
    b(state) = a(state);
    a(state) = T1 + T2;
}

static void sha512_block(sha512::state_type &state, sha512::block_t const &block) noexcept
{
    std::array<uint64_t,16> W;

    ttlet tmp_state = state;
    for (auto j = 0; j != 16; ++j) {
        sha512_round(
            tmp_state,
            K[j],
            W[j] = block[j]
        );
    }
    for (auto j = 16; j != 80; ++j) {
        sha512_round(
            tmp_state,
            K[j], 
            W[j & 0xf] =
                o1(W[(j- 2) & 0xf]) +
                   W[(j- 7) & 0xf] +
                o0(W[(j-15) & 0xf]) +
                   W[(j-16) & 0xf]
        );
    }
    state += tmp_state;
}

template<typename N>
std::array<std::byte,N> sha512(byte_string data)
{
    sha512::state_type state;
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

    return sha512_output<N>(state);
}

}
