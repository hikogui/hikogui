

#pragma once

#include <array>
#include <cstdint>

namespace tt {

class sha512 {
    struct state_t {
        uint64_t a;
        uint64_t b;
        uint64_t c;
        uint64_t d;
        uint64_t e;
        uint64_t f;
        uint64_t g;
        uint64_t h;

        [[nodiscard]] std::array<std::byte,64> get512() noexcept {
            std::array<std::byte,64> r;

            auto *ptr = r.data();
            get64(ptr, a); ptr += 8;
            get64(ptr, b); ptr += 8;
            get64(ptr, c); ptr += 8;
            get64(ptr, d); ptr += 8;
            get64(ptr, e); ptr += 8;
            get64(ptr, f); ptr += 8;
            get64(ptr, g); ptr += 8;
            get64(ptr, h);
            return r;
        }

        [[nodiscard]] std::array<std::byte,32> get256() noexcept {
            std::array<std::byte,32> r;

            auto *ptr = r.data();
            get64(ptr, a); ptr += 8;
            get64(ptr, b); ptr += 8;
            get64(ptr, c); ptr += 8;
            get64(ptr, d);
            return r;
        }

        static state_t sha512() {
            state_t r;
            r.a = 0x6a09e667f3bcc908;
            r.b = 0xbb67ae8584caa73b;
            r.c = 0x3c6ef372fe94f82b;
            r.d = 0xa54ff53a5f1d36f1;
            r.e = 0x510e527fade682d1;
            r.f = 0x9b05688c2b3e6c1f;
            r.g = 0x1f83d9abfb41bd6b;
            r.h = 0x5be0cd19137e2179;
            return r;
        }

        state_t(state_t const &other) noexcept
            a(other.a),
            b(other.b),
            c(other.c),
            d(other.d),
            e(other.e),
            f(other.f),
            g(other.g),
            h(other.h) {}

        state_t &operator=(state_t const &other) noexcept {
            a = other.a; 
            b = other.b; 
            c = other.c; 
            d = other.d; 
            e = other.e; 
            f = other.f; 
            g = other.g; 
            h = other.h; 
            return *this;
        }

        state_t &operator+=(state_t const &other) noexcept {
            a += other.a; 
            b += other.b; 
            c += other.c; 
            d += other.d; 
            e += other.e; 
            f += other.f; 
            g += other.g; 
            h += other.h; 
            return *this;
        }
    };

    using state_type = std::array<uint64_t,8>;
    using block_type = std::array<uint64_t,16>;

    state_type state;


};

}

