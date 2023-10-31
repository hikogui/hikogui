// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../container/container.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <bit>
#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <exception>
#include <string>

hi_export_module(hikogui.codec.SHA2);

hi_warning_push();
// C26429: Symbol '' is never tested for nullness, it can be marked as not_null (f.23)
// False positive reported: https://developercommunity.visualstudio.com/t/C26429-false-positive-on-reference-to-po/10262151
hi_warning_ignore_msvc(26429);

hi_export namespace hi { inline namespace v1 {

hi_export template<typename T, std::size_t Bits>
class SHA2 {
    static_assert(Bits % 8 == 0);
    constexpr static std::size_t nr_rounds = (sizeof(T) == 4) ? 64 : 80;
    constexpr static std::size_t pad_length_of_length = (sizeof(T) == 4) ? 8 : 16;

    struct state_type {
        T a;
        T b;
        T c;
        T d;
        T e;
        T f;
        T g;
        T h;

        constexpr state_type(T a, T b, T c, T d, T e, T f, T g, T h) noexcept : a(a), b(b), c(c), d(d), e(e), f(f), g(g), h(h) {}

        [[nodiscard]] constexpr T get_word(std::size_t i) const noexcept
        {
            switch (i) {
            case 0:
                return a;
            case 1:
                return b;
            case 2:
                return c;
            case 3:
                return d;
            case 4:
                return e;
            case 5:
                return f;
            case 6:
                return g;
            case 7:
                return h;
            default:
                hi_no_default();
            }
        }

        [[nodiscard]] constexpr std::byte get_byte(std::size_t i) const noexcept
        {
            hi_axiom(i < 8 * sizeof(T));
            hilet word_nr = i / sizeof(T);
            hilet byte_nr = i % sizeof(T);
            hilet word = get_word(word_nr);
            return static_cast<std::byte>(word >> (sizeof(T) - 1 - byte_nr) * 8);
        }

        template<std::size_t N>
        [[nodiscard]] bstring get_bytes() const noexcept
        {
            auto r = bstring{};
            r.reserve(N);

            for (std::size_t i = 0; i != N; ++i) {
                r += get_byte(i);
            }
            return r;
        }

        constexpr state_type& operator+=(state_type const& rhs) noexcept
        {
            a += rhs.a;
            b += rhs.b;
            c += rhs.c;
            d += rhs.d;
            e += rhs.e;
            f += rhs.f;
            g += rhs.g;
            h += rhs.h;
            return *this;
        }
    };

    struct block_type {
        std::array<T, 16> v;

        constexpr static std::size_t size = sizeof(v);

        constexpr void set_byte(std::size_t i, std::byte value) noexcept
        {
            hilet word_nr = i / sizeof(T);
            hilet byte_nr = i % sizeof(T);
            auto& word = v[word_nr];

            hilet valueT = static_cast<T>(static_cast<uint8_t>(value));
            word |= valueT << (sizeof(T) - 1 - byte_nr) * 8;
        }

        constexpr block_type(std::byte const *ptr) noexcept : v()
        {
            for (std::size_t i = 0; i != size; ++i) {
                set_byte(i, *(ptr++));
            }
        }

        constexpr T const& operator[](std::size_t i) const noexcept
        {
            return v[i % 16];
        }

        constexpr T& operator[](std::size_t i) noexcept
        {
            return v[i % 16];
        }
    };

    using byteptr = std::byte *;
    using cbyteptr = std::byte const *;
    state_type state;

    using overflow_type = std::array<std::byte, block_type::size>;
    overflow_type overflow;
    typename overflow_type::iterator overflow_it;

    std::size_t size;

    [[nodiscard]] constexpr static T K(std::size_t i) noexcept
    {
        constexpr std::array<uint32_t, 64> K32 = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

        constexpr std::array<uint64_t, 80> K64 = {
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
            0x431d67c49c100d4c, 0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817};

        if constexpr (std::is_same_v<T, uint32_t>) {
            return K32[i];
        } else {
            return K64[i];
        }
    }

    [[nodiscard]] constexpr static T Maj(T x, T y, T z) noexcept
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    [[nodiscard]] constexpr static T Ch(T x, T y, T z) noexcept
    {
        return (x & y) ^ (~x & z);
    }

    template<int A, int B, int C>
    [[nodiscard]] constexpr static T S(T x) noexcept
    {
        return std::rotr(x, A) ^ std::rotr(x, B) ^ std::rotr(x, C);
    }

    template<int A, int B, int C>
    [[nodiscard]] constexpr static T s(T x) noexcept
    {
        return std::rotr(x, A) ^ std::rotr(x, B) ^ x >> C;
    }

    [[nodiscard]] constexpr static T S0(T x) noexcept
    {
        if constexpr (std::is_same_v<T, uint32_t>) {
            return S<2, 13, 22>(x);
        } else {
            return S<28, 34, 39>(x);
        }
    }

    [[nodiscard]] constexpr static T S1(T x)
    {
        if constexpr (std::is_same_v<T, uint32_t>) {
            return S<6, 11, 25>(x);
        } else {
            return S<14, 18, 41>(x);
        }
    }

    [[nodiscard]] constexpr static T s0(T x)
    {
        if constexpr (std::is_same_v<T, uint32_t>) {
            return s<7, 18, 3>(x);
        } else {
            return s<1, 8, 7>(x);
        }
    }

    [[nodiscard]] constexpr static T s1(T x)
    {
        if constexpr (std::is_same_v<T, uint32_t>) {
            return s<17, 19, 10>(x);
        } else {
            return s<19, 61, 6>(x);
        }
    }

    constexpr static state_type round(state_type const& tmp, T K, T W) noexcept
    {
        hilet T1 = tmp.h + S1(tmp.e) + Ch(tmp.e, tmp.f, tmp.g) + K + W;

        hilet T2 = S0(tmp.a) + Maj(tmp.a, tmp.b, tmp.c);

        return {T1 + T2, tmp.a, tmp.b, tmp.c, tmp.d + T1, tmp.e, tmp.f, tmp.g};
    }

    constexpr void add(block_type W) noexcept
    {
        auto tmp = state;
        for (std::size_t i = 0; i != 16; ++i) {
            tmp = round(tmp, K(i), W[i]);
        }

        for (std::size_t i = 16; i != nr_rounds; ++i) {
            hilet W_ = s1(W[i - 2]) + W[i - 7] + s0(W[i - 15]) + W[i - 16];

            tmp = round(tmp, K(i), W_);

            W[i] = W_;
        }
        state += tmp;
    }

    constexpr void add_to_overflow(cbyteptr& ptr, std::byte const *last) noexcept
    {
        hi_axiom_not_null(ptr);
        hi_axiom_not_null(last);

        while (overflow_it != overflow.end() && ptr != last) {
            *(overflow_it++) = *(ptr++);
        }
    }

    constexpr void pad() noexcept
    {
        hi_axiom(overflow_it != overflow.end());

        // Add the terminating '1' bit.
        *(overflow_it++) = std::byte{0x80};

        // Complete the current block if there is not enough room
        // for the length in this block.
        hilet overflow_left = overflow.end() - overflow_it;
        if (overflow_left < pad_length_of_length) {
            while (overflow_it != overflow.end()) {
                *(overflow_it++) = std::byte{0x00};
            }
            add(block_type{overflow.data()});
            overflow_it = overflow.begin();
        }

        // Pad until the start of length.
        hilet overflow_length_start = overflow.end() - pad_length_of_length;
        while (overflow_it != overflow_length_start) {
            *(overflow_it++) = std::byte{0x00};
        }

        std::size_t nr_of_bits = size * 8;
        for (int i = pad_length_of_length - 1; i >= 0; --i) {
            *(overflow_it++) = i < sizeof(nr_of_bits) ? static_cast<std::byte>(nr_of_bits >> i * 8) : std::byte{0x00};
        }

        auto b = block_type{overflow.data()};
        add(b);
    }

public:
    constexpr SHA2(T a, T b, T c, T d, T e, T f, T g, T h) noexcept :
        state(a, b, c, d, e, f, g, h), overflow(), overflow_it(overflow.begin()), size(0)
    {
    }

    constexpr SHA2& add(std::byte const *ptr, std::byte const *last, bool finish = true) noexcept
    {
        size += last - ptr;

        if (overflow_it != overflow.begin()) {
            add_to_overflow(ptr, last);

            if (overflow_it == overflow.end()) {
                add(block_type{overflow.data()});
                overflow_it = overflow.begin();

            } else {
                if (finish) {
                    pad();
                }
                return *this;
            }
        }

        while (ptr + block_type::size <= last) {
            add(block_type{ptr});
            ptr += block_type::size;
        }

        add_to_overflow(ptr, last);

        if (finish) {
            pad();
        }
        return *this;
    }

    constexpr SHA2& add(bstring const& str, bool finish = true) noexcept
    {
        hilet first = str.data();
        hilet last = first + str.size();
        return add(first, last, finish);
    }

    constexpr SHA2& add(bstring_view str, bool finish = true) noexcept
    {
        hilet first = str.data();
        hilet last = first + str.size();
        return add(first, last, finish);
    }

    constexpr SHA2& add(std::string const& str, bool finish = true) noexcept
    {
        hilet first = reinterpret_cast<std::byte const *>(str.data());
        hilet last = first + str.size();
        return add(first, last, finish);
    }

    constexpr SHA2& add(std::string_view str, bool finish = true) noexcept
    {
        hilet first = reinterpret_cast<std::byte const *>(str.data());
        hilet last = first + str.size();
        return add(first, last, finish);
    }

    constexpr void add(std::span<std::byte const> str, bool finish = true) noexcept
    {
        hilet first = reinterpret_cast<std::byte const *>(str.data());
        hilet last = first + str.size();
        add(first, last, finish);
    }

    [[nodiscard]] bstring get_bytes() const noexcept
    {
        return state.template get_bytes<Bits / 8>();
    }
};

hi_export class SHA224 final : public SHA2<uint32_t, 224> {
public:
    SHA224() noexcept :
        SHA2<uint32_t, 224>(0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939, 0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4)
    {
    }
};

hi_export class SHA256 final : public SHA2<uint32_t, 256> {
public:
    SHA256() noexcept :
        SHA2<uint32_t, 256>(0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19)
    {
    }
};

hi_export class SHA384 final : public SHA2<uint64_t, 384> {
public:
    SHA384() noexcept :
        SHA2<uint64_t, 384>(
            0xcbbb9d5dc1059ed8,
            0x629a292a367cd507,
            0x9159015a3070dd17,
            0x152fecd8f70e5939,
            0x67332667ffc00b31,
            0x8eb44a8768581511,
            0xdb0c2e0d64f98fa7,
            0x47b5481dbefa4fa4)
    {
    }
};

hi_export class SHA512 final : public SHA2<uint64_t, 512> {
public:
    SHA512() noexcept :
        SHA2<uint64_t, 512>(
            0x6a09e667f3bcc908,
            0xbb67ae8584caa73b,
            0x3c6ef372fe94f82b,
            0xa54ff53a5f1d36f1,
            0x510e527fade682d1,
            0x9b05688c2b3e6c1f,
            0x1f83d9abfb41bd6b,
            0x5be0cd19137e2179)
    {
    }
};

hi_export class SHA512_224 final : public SHA2<uint64_t, 224> {
public:
    SHA512_224() noexcept :
        SHA2<uint64_t, 224>(
            0x8C3D37C819544DA2,
            0x73E1996689DCD4D6,
            0x1DFAB7AE32FF9C82,
            0x679DD514582F9FCF,
            0x0F6D2B697BD44DA8,
            0x77E36F7304C48942,
            0x3F9D85A86A1D36C8,
            0x1112E6AD91D692A1)
    {
    }
};

hi_export class SHA512_256 final : public SHA2<uint64_t, 256> {
public:
    SHA512_256() noexcept :
        SHA2<uint64_t, 256>(
            0x22312194FC2BF72C,
            0x9F555FA3C84C64C2,
            0x2393B86B6F53B151,
            0x963877195940EABD,
            0x96283EE2A88EFFE3,
            0xBE5E1E2553863992,
            0x2B0199FC2C85B8AA,
            0x0EB72DDC81C52CA2)
    {
    }
};

}} // namespace hi::v1

hi_warning_pop();
