// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../strings.hpp"
#include "../unfair_mutex.hpp"
#include "../cast.hpp"
#include <cstdint>
#include <string>
#include <string_view>
#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>
#include <bit>

namespace hi::inline v1 {
namespace detail {

class long_grapheme {
public:
    constexpr long_grapheme() noexcept = default;
    constexpr long_grapheme(size_t size) noexcept : _size(size){};
    long_grapheme(long_grapheme const&) = delete;
    long_grapheme(long_grapheme&&) = delete;
    long_grapheme& operator=(long_grapheme const&) = delete;
    long_grapheme& operator=(long_grapheme&&) = delete;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _size == 0;
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _size;
    }

    [[nodiscard]] char32_t *data() noexcept
    {
        return std::launder(reinterpret_cast<char32_t *>(this + 1));
    }

    [[nodiscard]] char32_t const *data() const noexcept
    {
        return std::launder(reinterpret_cast<char32_t const *>(this + 1));
    }

    [[nodiscard]] char32_t *begin() noexcept
    {
        return data();
    }

    [[nodiscard]] char32_t const *begin() const noexcept
    {
        return data();
    }

    [[nodiscard]] char32_t *end() noexcept
    {
        return data() + size();
    }

    [[nodiscard]] char32_t const *end() const noexcept
    {
        return data() + size();
    }

    [[nodiscard]] char32_t& operator[](size_t index) noexcept
    {
        return *(data() + index);
    }

    [[nodiscard]] char32_t const& operator[](size_t index) const noexcept
    {
        return *(data() + index);
    }

    template<size_t I>
    [[nodiscard]] friend char32_t& get(long_grapheme& rhs) noexcept
    {
        return *(rhs.data() + I);
    }

    template<size_t I>
    [[nodiscard]] friend char32_t const& get(long_grapheme const& rhs) noexcept
    {
        return *(rhs.data() + I);
    }

    [[nodiscard]] bool equal(std::u32string_view rhs) const noexcept
    {
        return std::equal(begin(), end(), rhs.begin(), rhs.end());
    }

    [[nodiscard]] bool lexicographical_compare(std::u32string_view rhs) const noexcept
    {
        return std::lexicographical_compare(begin(), end(), rhs.begin(), rhs.end());
    }

private:
    size_t _size = 0;
};

} // namespace detail

/** A grapheme, what a user thinks a character is.
 *
 * This class will hold:
 * - empty/eof (no code points encoded at all, the value 0)
 * - U+0000 (value 1)
 * - 1 starter code-point, followed by 0-4 combining characters.
 *
 * A grapheme should not include typographical ligatures such as 'fi' as
 * the font should handle creating ligatures.
 *
 * If a grapheme is initialized with more than 3 code-points a long_grapheme
 * is allocated. This grapheme is never deleted from memory.
 *
 * This class is trivial and constant-destructible so that it can be used
 * as a character class in `std::basic_string` and used as a non-type template parameter.
 *
 */
struct grapheme {
    using ptr_type = detail::long_grapheme const *;
    using value_type = uintptr_t;

    static_assert(sizeof(value_type) == sizeof(ptr_type));

    /** A pointer to a grapheme.
     *
     * - nullptr : Empty grapheme
     * - pointer : A pointer to a long grapheme (more than 3 code-points).
     * - short-grapheme optimization:
     *   + [63:43] 3rd code-point (or U+0000 if only 2 code-points)
     *   + [42:22] 2nd code-point (or U+0000 if only 1 code-point)
     *   + [21:1] 1st code-point (may be U+0000)
     *   + [0] '1' (invalid aligned pointer).
     *
     * The above works because there can only be one U+0000 character in a grapheme
     * and it must be the first.
     */
    value_type value;

    constexpr grapheme() noexcept = default;
    constexpr grapheme(grapheme const&) noexcept = default;
    constexpr grapheme(grapheme&&) noexcept = default;
    constexpr grapheme& operator=(grapheme const&) noexcept = default;
    constexpr grapheme& operator=(grapheme&&) noexcept = default;

    [[nodiscard]] ptr_type pointer() const noexcept
    {
        return std::launder(std::bit_cast<ptr_type>(value));
    }

    /** Encode a single code-point.
     */
    constexpr explicit grapheme(char32_t code_point) noexcept : value((static_cast<value_type>(code_point) << 1) | 1)
    {
        hi_axiom(code_point <= 0x10ffff and code_point != 0xffff);
    }

    constexpr explicit grapheme(char ascii_char) noexcept : value((static_cast<value_type>(ascii_char) << 1) | 1)
    {
        hi_axiom(ascii_char >= 0 and ascii_char <= 0x7f);
    }

    /** Encode a single code-point.
     */
    constexpr grapheme& operator=(char32_t code_point) noexcept
    {
        value = (static_cast<value_type>(code_point) << 1) | 1;
        return *this;
    }

    /** Encode a single code-point.
     */
    constexpr grapheme& operator=(char ascii_char) noexcept
    {
        hi_axiom(ascii_char >= 0 and ascii_char <= 0x7f);
        value = (static_cast<value_type>(ascii_char) << 1) | 1;
        return *this;
    }

    /** Encode a grapheme from a list of code-points.
     *
     * @param code_points The non-normalized list of code-points.
     */
    explicit grapheme(std::u32string_view code_points) noexcept;

    /** Encode a grapheme from a list of code-points.
     *
     * @param code_points The non-normalized list of code-points.
     */
    grapheme& operator=(std::u32string_view code_points) noexcept;

    /** Encode a grapheme from a list of NFKC-normalized code-points.
     *
     * @param code_points The NFKC-normalized list of code-points.
     */
    [[nodiscard]] static grapheme from_composed(std::u32string_view code_points) noexcept;

    /** Create empty grapheme / end-of-file.
     */
    [[nodiscard]] static constexpr grapheme eof() noexcept
    {
        grapheme r;
        r.value = 0;
        return r;
    }

    /** Clear the grapheme.
     */
    constexpr void clear() noexcept
    {
        value = 0;
    }

    /** Check if the grapheme is empty.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return value == 0;
    }

    /** Check if the grapheme holds any code-points.
     */
    constexpr operator bool() const noexcept
    {
        return not empty();
    }

    /** Check if the grapheme is valid.
     *
     * A grapheme is invalid in case:
     * - The grapheme is empty.
     * - The first code-point is part of general category 'C'.
     * - The first code-point is a combining character; canonical combining class != 0.
     */
    [[nodiscard]] bool valid() const noexcept;

    /** Return the number of code-points encoded in the grapheme.
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        if (value == 0) {
            return 0;

        } else if (value & 1) {
            auto tmp = value;
            if (not(tmp >>= 22)) {
                return 1;
            }
            if (not(tmp >>= 21)) {
                return 2;
            }
            return 3;

        } else {
            return pointer()->size();
        }
    }

    /** Get the code-point at the given index.
     *
     * @note It is undefined-behavior to index beyond the number of encoded code-points.
     * @param i Index of code-point in the grapheme.
     * @return code-point at the given index.
     */
    [[nodiscard]] constexpr char32_t operator[](size_t i) const noexcept
    {
        hi_axiom(i < size());

        if (value & 1) {
            return (value >> (1 + (i * 21))) & 0x1f'ffff;
        } else {
            return (*pointer())[i];
        }
    }

    /** Get the code-point at the given index.
     *
     * @note It is undefined-behavior to index beyond the number of encoded code-points.
     * @tparam I Index of code-point in the grapheme.
     * @param rhs The grapheme to query.
     * @return code-point at the given index.
     */
    template<size_t I>
    [[nodiscard]] friend constexpr char32_t get(grapheme const& rhs) noexcept
    {
        hi_axiom(I < rhs.size());

        if (rhs.value & 1) {
            return (rhs.value >> (1 + (I * 21))) & 0x1f'ffff;
        } else {
            return get<I>(*rhs.pointer());
        }
    }

    /** Get a list of code-point normalized to NFC.
     */
    [[nodiscard]] constexpr std::u32string composed() const noexcept
    {
        auto r = std::u32string{};
        r.reserve(size());
        for (std::size_t i = 0; i != size(); ++i) {
            r += (*this)[i];
        }
        return r;
    }

    /** Get a list of code-point normalized to NFD.
     */
    [[nodiscard]] std::u32string decomposed() const noexcept;

    // The default equality works in combination with long-graphemes, since long-graphemes have stable and unique pointer values.
    /** Compare equivalence of two graphemes.
     */
    [[nodiscard]] friend constexpr bool operator==(grapheme const&, grapheme const&) noexcept = default;

    /** Compare two graphemes lexicographically.
     */
    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const& lhs, grapheme const& rhs) noexcept
    {
        if (lhs.value & rhs.value & 1) {
            return lhs.value <=> rhs.value;
        }

        hilet size = std::min(lhs.size(), rhs.size());

        for (auto i = 0_uz; i != size; ++i) {
            if (hilet r = lhs[i] <=> rhs[i]; r != std::strong_ordering::equal) {
                return r;
            }
        }
        return lhs.size() <=> rhs.size();
    }

    [[nodiscard]] friend constexpr bool operator==(grapheme const& lhs, char32_t const& rhs) noexcept
    {
        return lhs == grapheme{rhs};
    }

    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const& lhs, char32_t const& rhs) noexcept
    {
        return lhs <=> grapheme{rhs};
    }

    [[nodiscard]] friend constexpr bool operator==(grapheme const& lhs, char const& rhs) noexcept
    {
        return lhs == grapheme{rhs};
    }

    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const& lhs, char const& rhs) noexcept
    {
        return lhs <=> grapheme{rhs};
    }

    [[nodiscard]] friend std::string to_string(grapheme const& rhs) noexcept
    {
        return hi::to_string(rhs.composed());
    }

    [[nodiscard]] friend std::u32string to_u32string(grapheme const& rhs) noexcept
    {
        return rhs.composed();
    }
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::grapheme> {
    [[nodiscard]] std::size_t operator()(hi::grapheme const& rhs) const noexcept
    {
        return std::hash<hi::grapheme::value_type>{}(rhs.value);
    }
};
