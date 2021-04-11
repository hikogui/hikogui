// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../strings.hpp"
#include "../cast.hpp"
#include "../hash.hpp"
#include <array>

namespace tt {

// "Compatibility mappings are guaranteed to be no longer than 18 characters, although most consist of just a few characters."
// https://unicode.org/reports/tr44/ (TR44 5.7.3)
using long_grapheme = std::array<char32_t, 18>;

/*! A grapheme, what a user thinks a character is.
 * This will exclude ligatures, because a user would see those as separate characters.
 */
class grapheme {
    /*! This value contains up to 3 code-points, or a pointer+length to an array
     * of code-points located on the heap.
     *
     * The code-points inside the grapheme are in NFC.
     *
     * If bit 0 is '1' the value contains up to 3 code-points as follows:
     *    - 63:43   3rd code-point, or zero
     *    - 42:22   2nd code-point, or zero
     *    - 21:1    1st code-point, or zero
     *    - 0       '1'
     *
     * if bit 0 is '0' the value contains a length+pointer as follows:
     *    - 63:48   Length
     *    - 47:0    Pointer to long_grapheme on the heap;
     *              bottom two bits are zero, due to alignment.
     */
    uint64_t value;

public:
    grapheme() noexcept : value(1) {}

    ~grapheme()
    {
        delete_pointer();
    }

    grapheme(const grapheme &other) noexcept
    {
        tt_axiom(&other != this);
        value = other.value;
        if (other.has_pointer()) {
            value = create_pointer(other.get_pointer()->data(), other.size());
        }
    }

    grapheme &operator=(const grapheme &other) noexcept
    {
        tt_short_circuit_self_assignment(other);
        delete_pointer();
        value = other.value;
        if (other.has_pointer()) {
            value = create_pointer(other.get_pointer()->data(), other.size());
        }
        return *this;
    }

    grapheme(grapheme &&other) noexcept
    {
        tt_axiom(&other != this);
        value = other.value;
        other.value = 1;
    }

    grapheme &operator=(grapheme &&other) noexcept
    {
        // Self-assignment is allowed.
        delete_pointer();
        value = other.value;
        other.value = 1;
        return *this;
    }

    explicit grapheme(std::u32string_view codePoints) noexcept;

    explicit grapheme(char32_t codePoint) noexcept : grapheme(std::u32string_view{&codePoint, 1}) {}

    template<typename It>
    explicit grapheme(It ptr, It last) noexcept : grapheme(*ptr)
    {
        ++ptr;
        while (ptr != last) {
            *this += *(ptr++);
        }
    }

    grapheme &operator=(std::u32string_view codePoints) noexcept
    {
        *this = grapheme(codePoints);
        return *this;
    }

    grapheme &operator=(char32_t codePoint) noexcept
    {
        *this = grapheme(codePoint);
        return *this;
    }

    grapheme &operator+=(char32_t codePoint) noexcept;

    explicit operator std::u32string() const noexcept
    {
        if (has_pointer()) {
            return {get_pointer()->data(), size()};
        } else {
            auto r = std::u32string{};
            auto tmp = value >> 1;
            for (size_t i = 0; i < 3; i++, tmp >>= 21) {
                if (auto codePoint = static_cast<char32_t>(tmp & 0x1f'ffff)) {
                    r += codePoint;
                } else {
                    return r;
                }
            }
            return r;
        }
    }

    operator bool() const noexcept
    {
        return value != 1;
    }

    [[nodiscard]] size_t hash() const noexcept
    {
        size_t r = 0;
        for (ssize_t i = 0; i != std::ssize(*this); ++i) {
            r = hash_mix_two(r, std::hash<char32_t>{}((*this)[i]));
        }
        return r;
    }

    [[nodiscard]] size_t size() const noexcept
    {
        if (has_pointer()) {
            return value >> 48;
        } else {
            auto tmp = value >> 1;
            size_t i;
            for (i = 0; i < 3; i++, tmp >>= 21) {
                if ((tmp & 0x1f'ffff) == 0) {
                    return i;
                }
            }
            return i;
        }
    }

    [[nodiscard]] char32_t front() const noexcept
    {
        if (size() == 0) {
            return 0;
        } else {
            return (*this)[0];
        }
    }

    [[nodiscard]] char32_t operator[](size_t i) const noexcept
    {
        if (has_pointer()) {
            tt_axiom(i < std::tuple_size_v<long_grapheme>);
            return (*get_pointer())[i];

        } else {
            tt_axiom(i < 3);
            return (value >> ((i * 21) + 1)) & 0x1f'ffff;
        }
    }

    [[nodiscard]] std::u32string NFC() const noexcept
    {
        std::u32string r;
        r.reserve(std::ssize(*this));
        for (ssize_t i = 0; i != std::ssize(*this); ++i) {
            r += (*this)[i];
        }
        return r;
    }

    [[nodiscard]] std::u32string NFD() const noexcept;

    [[nodiscard]] std::u32string NFKC() const noexcept;

    [[nodiscard]] std::u32string NFKD() const noexcept;

    /** Paragraph separator.
     */
    static grapheme PS() noexcept
    {
        return grapheme(U'\u2029');
    }

    /** Line separator.
     */
    static grapheme LS() noexcept
    {
        return grapheme(U'\u2028');
    }

    [[nodiscard]] friend std::string to_string(grapheme const &g) noexcept
    {
        return tt::to_string(g.NFC());
    }

    [[nodiscard]] friend std::u8string to_u8string(grapheme const &g) noexcept
    {
        return tt::to_u8string(g.NFC());
    }

    friend std::ostream &operator<<(std::ostream &lhs, grapheme const &rhs)
    {
        return lhs << to_string(rhs);
    }

private:
    [[nodiscard]] bool has_pointer() const noexcept
    {
        return (value & 1) == 0;
    }

    [[nodiscard]] static uint64_t create_pointer(char32_t const *data, size_t size) noexcept
    {
        tt_assert(size <= std::tuple_size<long_grapheme>::value);

        auto ptr = new long_grapheme();
        memcpy(ptr->data(), data, size);

        auto iptr = reinterpret_cast<ptrdiff_t>(ptr);
        auto uptr = static_cast<uint64_t>(iptr << 16) >> 16;
        return (size << 48) | uptr;
    }

    [[nodiscard]] long_grapheme *get_pointer() const noexcept
    {
        auto uptr = (value << 16);
        auto iptr = static_cast<ptrdiff_t>(uptr) >> 16;
        return std::launder(reinterpret_cast<long_grapheme *>(iptr));
    }

    void delete_pointer() noexcept
    {
        if (has_pointer()) {
            delete get_pointer();
        }
    }

    [[nodiscard]] friend bool operator<(grapheme const &a, grapheme const &b) noexcept
    {
        ttlet length = std::min(std::ssize(a), std::ssize(b));

        for (ssize_t i = 0; i != length; ++i) {
            if (a[i] < b[i]) {
                return true;
            }
        }
        return std::ssize(a) < std::ssize(b);
    }

    [[nodiscard]] friend bool operator==(grapheme const &a, grapheme const &b) noexcept
    {
        if (a.value == b.value) {
            return true;
        }

        if (std::ssize(a) != std::ssize(b)) {
            return false;
        }

        for (ssize_t i = 0; i != std::ssize(a); ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] friend bool operator!=(grapheme const &a, grapheme const &b) noexcept
    {
        return !(a == b);
    }

    [[nodiscard]] friend bool operator==(grapheme const &lhs, char32_t const &rhs) noexcept
    {
        return (std::ssize(lhs) == 1) && (lhs[0] == rhs);
    }

    [[nodiscard]] friend bool operator!=(grapheme const &lhs, char32_t const &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend bool operator==(grapheme const &lhs, char const &rhs) noexcept
    {
        return lhs == static_cast<char32_t>(rhs);
    }

    [[nodiscard]] friend bool operator!=(grapheme const &lhs, char const &rhs) noexcept
    {
        return !(lhs == rhs);
    }
};

} // namespace tt

namespace std {

template<>
struct hash<tt::grapheme> {
    [[nodiscard]] size_t operator()(tt::grapheme const &rhs) const noexcept
    {
        return rhs.hash();
    }
};

} // namespace std
