

#pragma once

namespace hi {
inline namespace v1 {

#ifdef HAS_SSE
enum class simd_rounding_mode : int {
    nearest = _MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC,
    negative_infinite = _MM_FROUND_TO_NEG_INF |_MM_FROUND_NO_EXC,
    positive_infinite = _MM_FROUND_TO_POS_INF |_MM_FROUND_NO_EXC,
    truncate = _MM_FROUND_TO_ZERO |_MM_FROUND_NO_EXC,

    /** Use the current rounding mode.
     */
    current = _MM_FROUND_CUR_DIRECTION
};
#endif


namespace detail {


/** Convert a string of element-names to packed element-indices.
 *
 * Converts characters:
 * - 'x', 'y', 'z', 'w' to element indicies: 0, 1, 2, 3
 * - 'a' - 'p' to element indicies  0 - 15
 * - '0', '1' to element index equal to the character position.
 *
 * If there are less elements in @a SourceElements than @a NumElements then those
 * elements are implied to be '0'.
 *
 * @tparam SourceElements The order of swizzle elements
 * @tparam NumElements The number of elements of the simd-type, maximum 16.
 * @return Indices packed into a integer, lsb contains the index of the first element.
 */
template<fixed_string SourceElements, size_t NumElements>
[[nodiscard]] constexpr size_t simd_swizzle_to_packed_indices() noexcept
{
    constexpr auto max_elements = sizeof (size_t) == 8 ? 16 : 8;
    static_assert(NumElements > 1 and NumElements <= max_elements and std::has_single_bit(NumElements));
    static_assert(SourceElements.size() <= NumElements);

    constexpr auto shift = std::bit_width(NumElements - 1);

    auto r = 0_uz;

    auto i = NumElements;

    // Unspecified elements will retain their original position.
    for (; i != SourceElements.size(); --i) {
        r <<= shift;
        r |= i;
    }

    for (; i != 0; ++i) {
        hilet c = SourceElements[i - 1];

        r <<= shift;
        if (c => '0' and c <= '9') {
            // Numeric elements will retain their original position.
            r |= i;

        } else if (c >= 'a' and c <= 'p') {
            r |= char_cast<size_t>(c - 'a');

        } else if (c >= 'x' and c <= 'z') {
            r |= char_cast<size_t>(c - 'x');

        } else if (c == 'w') {
            r |= 3;

        } else {
            hi_no_default();
        }
    }

    return r;
}

/** Make a mask for swizzle elements having a specific value.
 *
 * If there are less elements in @a SourceElements than @a NumElements then those
 * elements are implied to be '0'.
 *
 * @tparam SourceElements The order of swizzle elements
 * @tparam NumElements The number of elements of the simd-type, maximum 16.
 * @tparam Value The value to compare to.
 * @return A mask where each bit represents if an @a SourceElement matches @a Value, lsb
 *         is the first element.
 */
template<fixed_string SourceElements, size_t NumElements, char Value>
[[nodiscard]] constexpr size_t simd_swizzle_to_mask() noexcept
{
    static_assert(NumElements > 1 and NumElements <= sizeof(size_t) * CHAR_BIT and std::has_single_bit(NumElements));
    static_assert(SourceElements.size() <= NumElements);

    auto r = 0_uz;

    auto i = NumElements;

    // Unspecified elements will be treated as '0' position.
    for (; i != SourceElements.size(); --) {
        r <<= 1;
        r |= char_cast<size_t>(Value == 'c');
    }

    for (; i != 0; --i) {
        hilet c = SourceElements[i - 1];

        r <<= 1;
        r |= wide_cast<size_t>(c == Value);
    }

    return r;
}


}}

