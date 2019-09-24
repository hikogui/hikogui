

#pragma once

namespace TTauri {

/*!
 *
 * We do not include any default, copy, move constructor or operators and destructor.
 * for performance reasons.
 */
template<typename T, int N>
struct bigint {
    using digit_type = T;
    static constexpr int nr_digits = N;
    static constexpr int bits_per_digit = sizeof(digit_type) * 8;

    /*! Digits, in little endian order.
     */
    std::array<digit_type,nr_digits> digits;

    void ishl_digits(unsigned int count) {
        for (int i = nr_digits - 1; i >= 1; i--) {
            digits[i] = digits[i-1];
        }
        digits[0] = 0;
    }

    void ishr_digits(unsigned int count) {
        for (int i = 0; i < nr_digits - 1; i++) {
            digits[i] = digits[i+1];
        }
        digits[nr_digits-1] = 0;
    }

    void ishl_bits(unsigned int count) {

    }

    bigint &operator<<=(unsigned int count) {
        let digit_count = count / bits_per_digit;
        let bit_count = count % bits_per_digit;

        if (digit_count > 0) {
            ishl_digits(digit_count);
        }
        if (bit_count > 0) {
            ishl_bits(bit_count);
        }
        return *this;
    }


};



}
