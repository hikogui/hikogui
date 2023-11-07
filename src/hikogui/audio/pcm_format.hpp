// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <bit>
#include <cstdint>
#include <string>
#include <format>

hi_export_module(hikogui.audio.pcm_format);

hi_export namespace hi { inline namespace v1 {

hi_export class pcm_format {
public:
    constexpr pcm_format() noexcept = default;
    constexpr pcm_format(pcm_format&&) noexcept = default;
    constexpr pcm_format(pcm_format const&) noexcept = default;
    constexpr pcm_format& operator=(pcm_format&&) noexcept = default;
    constexpr pcm_format& operator=(pcm_format const&) noexcept = default;
    [[nodiscard]] friend constexpr bool operator==(pcm_format const&, pcm_format const&) noexcept = default;

    [[nodiscard]] friend constexpr auto operator<=>(pcm_format const &lhs, pcm_format const &rhs) noexcept
    {
        if (hilet tmp = lhs._floating_point <=> rhs._floating_point; tmp != std::strong_ordering::equal) {
            return tmp;
        }
        if (hilet tmp = lhs._num_major_bits <=> rhs._num_major_bits; tmp != std::strong_ordering::equal) {
            return tmp;
        }
        if (hilet tmp = lhs._num_minor_bits <=> rhs._num_minor_bits; tmp != std::strong_ordering::equal) {
            return tmp;
        }
        if (hilet tmp = lhs._num_bytes <=> rhs._num_bytes; tmp != std::strong_ordering::equal) {
            return tmp;
        }
        if (hilet tmp = lhs._lsb <=> rhs._lsb; tmp != std::strong_ordering::equal) {
            return tmp;
        }
        return lhs._lsb <=> rhs._lsb;
    }

    [[nodiscard]] constexpr friend bool equal_except_bit_depth(pcm_format const& lhs, pcm_format const& rhs) noexcept
    {
        return lhs._floating_point == rhs._floating_point;
    }

    /** Construct a PCM format.
     *
     * @param floating_point True when format is in floating-point, false if format is in fixed-point/signed-integer.
     * @param endian The storage endianness.
     * @param lsb True when the sample is aligned to the least-significant-bits (LSB) of the storage, false is in
     *            most-significant-bits (MSB).
     * @param num_bytes The number of bytes of storage for the sample. Between 1 up to and including 8 bytes.
     * @param num_major_bits The number of exponent (floating-point) or integral (fixed-point) bits. Between 0 up to and including
     *                       15 bits. Set to 0 for signed-integer samples.
     * @param num_minor_bits The number of mantissa (floating-point) or fractional (fixed-point) bits. Between 1 up to and
     * including 63 bits. Set to the number of bits excluding sign-bit for signed-integer samples.
     */
    constexpr pcm_format(
        bool floating_point,
        std::endian endian,
        bool lsb,
        uint8_t num_bytes,
        uint8_t num_major_bits,
        uint8_t num_minor_bits) noexcept :
        _floating_point(floating_point),
        _little_endian(endian == std::endian::little),
        _lsb(lsb),
        _num_bytes(num_bytes - 1),
        _num_major_bits(num_major_bits),
        _num_minor_bits(num_minor_bits)
    {
        hi_assert(num_bytes >= 1 and num_bytes <= 8);
        hi_assert(num_major_bits >= 0 and num_major_bits <= 15);
        hi_assert(num_minor_bits >= 1 and num_minor_bits <= 63);
    }

    [[nodiscard]] constexpr static pcm_format float32() noexcept
    {
        return pcm_format{true, std::endian::native, true, 4, 8, 23};
    }

    [[nodiscard]] constexpr static pcm_format float32_le() noexcept
    {
        return pcm_format{true, std::endian::little, true, 4, 8, 23};
    }

    [[nodiscard]] constexpr static pcm_format float32_be() noexcept
    {
        return pcm_format{true, std::endian::big, true, 4, 8, 23};
    }

    [[nodiscard]] constexpr static pcm_format sint24() noexcept
    {
        return pcm_format{false, std::endian::native, true, 3, 0, 23};
    }

    [[nodiscard]] constexpr static pcm_format sint24_le() noexcept
    {
        return pcm_format{false, std::endian::little, true, 3, 0, 23};
    }

    [[nodiscard]] constexpr static pcm_format sint24_be() noexcept
    {
        return pcm_format{false, std::endian::big, true, 3, 0, 23};
    }

    [[nodiscard]] constexpr static pcm_format sint16() noexcept
    {
        return pcm_format{false, std::endian::native, true, 2, 0, 15};
    }

    [[nodiscard]] constexpr static pcm_format sint16_le() noexcept
    {
        return pcm_format{false, std::endian::little, true, 2, 0, 15};
    }

    [[nodiscard]] constexpr static pcm_format sint16_be() noexcept
    {
        return pcm_format{false, std::endian::big, true, 2, 0, 15};
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _num_minor_bits == 0;
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr bool floating_point() const noexcept
    {
        hi_axiom(not empty());
        return _floating_point;
    }

    [[nodiscard]] constexpr bool fixed_point() const noexcept
    {
        hi_axiom(not empty());
        return not floating_point();
    }

    [[nodiscard]] constexpr std::endian endian() const noexcept
    {
        hi_axiom(not empty());
        return _little_endian ? std::endian::little : std::endian::big;
    }

    /** The number of bytes a sample is stored in.
     *
     * This value determines the 'storage'
     */
    uint8_t num_bytes() const noexcept
    {
        hi_axiom(not empty());
        return narrow_cast<uint8_t>(_num_bytes + 1);
    }

    /** The sample is stored in the least-significant-bits of the storage.
     */
    [[nodiscard]] constexpr bool lsb() const noexcept
    {
        hi_axiom(not empty());
        return _lsb;
    }

    /** The sample is stored in the most-significant-bits of the storage.
     */
    [[nodiscard]] constexpr bool msb() const noexcept
    {
        hi_axiom(not empty());
        return not lsb();
    }

    /** The number of least significant bits of the storage that is used by the sample.
     *
     * This value determines the alignment of the sample within the storage. The value
     * includes all the significant bits of a sample including the sign bit.
     *
     * This function will return:
     *  - 8 for a 'floating-point 32 bit PCM' sample format.
     */
    uint8_t num_bits() const noexcept
    {
        hi_axiom(not empty());
        return narrow_cast<uint8_t>(_num_major_bits + _num_minor_bits + 1);
    }

    /** The number of bits in the exponent.
     *
     * This function will return:
     *  - 32 for a 'floating-point 32 bit PCM' sample format.
     *  - 32 for a 'fixed point Q7.24 PCM / iOS CoreAudio 8.24' format.
     *  - 24 for a 'signed integer 24 PCM' format.
     *
     * @note It is undefined behavior to call this function on a fixed-point sample format.
     */
    [[nodiscard]] uint8_t num_exponent_bits() const noexcept
    {
        hi_axiom(floating_point());
        return _num_major_bits;
    }

    /** The number of bits in the mantissa.
     *
     * This function will return:
     *  - 23 for a 'floating-point 32 bit PCM' sample format.
     *
     * @note It is undefined behavior to call this function on a fixed-point sample format.
     */
    [[nodiscard]] uint8_t num_mantissa_bits() const noexcept
    {
        hi_axiom(floating_point());
        return _num_minor_bits;
    }

    /** The number of integral bits.
     *
     * In fixed point format these are the number of bits for allowing the
     * sample to overflow above 1.0 or below -1.0.
     *
     * This function will return:
     *  - 7 for a 'fixed point Q7.24 PCM / iOS CoreAudio 8.24' format.
     *  - 0 for a 'signed integer 24 PCM' format.
     *
     * @note It is undefined-behavior to call this function on floating-point sample formats.
     */
    [[nodiscard]] uint8_t num_integral_bits() const noexcept
    {
        hi_axiom(fixed_point());
        return _num_major_bits;
    }

    /** The number of fractional bits.
     *
     * In fixed point format these are the number of fractional bits.
     *
     * For signed-integer formats this value is the number of bits, excluding the sign-bit.
     * This function will return:
     *  - 24 for a 'fixed point Q7.24 PCM / iOS CoreAudio 8.24' format.
     *  - 23 for a 'signed integer 24 PCM' format.
     *
     * @note It is undefined-behavior to call this function on floating-point sample formats.
     */
    [[nodiscard]] uint8_t num_fraction_bits() const noexcept
    {
        hi_axiom(fixed_point());
        return _num_minor_bits;
    }

    [[nodiscard]] friend std::string to_string(pcm_format const& rhs) noexcept
    {
        if (rhs.floating_point()) {
            if (rhs.endian() == std::endian::native) {
                return std::format("float-{}", rhs.num_bits());
            } else {
                return std::format("float-{}_{}", rhs.num_bits(), rhs.endian() == std::endian::little ? "le" : "be");
            }

        } else if (rhs.num_integral_bits() == 0) {
            if (rhs.endian() == std::endian::native) {
                return std::format("int-{}", rhs.num_bits());
            } else {
                return std::format("int-{}_{}", rhs.num_bits(), rhs.endian() == std::endian::little ? "le" : "be");
            }

        } else {
            if (rhs.endian() == std::endian::native) {
                return std::format("Q{}.{}", rhs.num_integral_bits(), rhs.num_fraction_bits());
            } else {
                return std::format(
                    "Q{}.{}_{}",
                    rhs.num_integral_bits(),
                    rhs.num_fraction_bits(),
                    rhs.endian() == std::endian::little ? "le" : "be");
            }
        }
    }

private:
    uint16_t _floating_point : 1 = 0;
    uint16_t _little_endian : 1 = 0;
    uint16_t _lsb : 1 = 0;
    uint16_t _num_bytes : 3 = 0;
    uint16_t _num_major_bits : 4 = 0;
    uint16_t _num_minor_bits : 6 = 0;
};

}} // namespace hi::inline v1

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::pcm_format, char> : std::formatter<std::string_view, char> {
    auto format(hi::pcm_format const& t, auto& fc) const
    {
        return std::formatter<std::string_view, char>::format(to_string(t), fc);
    }
};
