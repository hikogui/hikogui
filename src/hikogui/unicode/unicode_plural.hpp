
#pragma once

namespace hi {
inline namespace v1 {

enum class plurality_value : uint8_t { zero = 0, one = 1, two = 2, few = 3, many = 4, other = 5 };

enum class plurality_mask : uint8_t {
    zero = 1 << to_underlying(plurality::zero),
    one = 1 << to_underlying(plurality::one),
    two = 1 << to_underlying(plurality::two),
    few = 1 << to_underlying(plurality::few),
    many = 1 << to_underlying(plurality::many),
    other = 1 << to_underlying(plurality::other),
};

[[nodiscard]] constexpr plurality_mask operator|(plurality_mask const& lhs, plurality_mask const& rhs) noexcept
{
    return static_cast<plurality_mask>(to_underlying(lhs) | to_underlying(rhs));
}

struct plurality {
    plurality_value value;
    plurality_mask mask;

    /** Get an index to select between translations.
     *
     * @param n The number of plural messages for this translation.
     * @return The index into the plural message table for this translation.
     *         If there are not enough messages, then the index to the last message
     *         is returned.
     */
    [[nodiscard]] constexpr size_t index(size_t n) const noexcept
    {
        hi_assert(n != 0);

        hilet value_as_mask = (1 << (to_underlying(value) + 1)) - 1;
        // Get the index based on the number of '1' bits that are set from the
        // plurality position to lsb.
        hilet i = std::popcount(value_as_mask & to_underlying(mask)) - 1;
        if (i < n) {
            return i;
        } else {
            return n - 1;
        }
    }
};

class plural_rule {
public:
    [[nodiscard]] virtual plurality cardinal(operand_type op) noexcept
    {
        return {plurality_value::other, plurality_mask::other};
    }

    [[nodiscard]] plurality cardinal(long long n) noexcept
    {
        return cardinal(operand_type(n)).index(size);
    }

    [[nodiscard]] size_t cardinal(long long n, size_t size) noexcept
    {
        return cardinal(n).index(size);
    }

private:
    struct operand_type {
        /** Absolute value.
         */
        unsigned long long n = 0;

        /** Visible fraction digits with trailing zeros.
         */
        unsigned long long f = 0;

        /** Visible fraction digits without trailing zeros.
         */
        unsigned long long t = 0;

        /** Number of digits in the value.
         */
        uint8_t i = 0;

        /** Number of visible fraction digits with trailing zeros.
         */
        uint8_t v = 0;

        /** Number of visible fraction digits without trailing zeros.
         */
        uint8_t w = 0;

        /** Compact decimal exponent value.
         */
        uint8_t c = 0;

        constexpr operand_type(long long value) noexcept : n(std::abs(value)), i(decimal_width(value)) {}
    };
};

}}

