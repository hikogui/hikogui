

namespace tt {

[[nodiscard]] tt_force_inline i8x16 load_audio_samples(
    std::byte const *&src,
    int stride,
    i8x16 byte_order
) noexcept
{
    auto bytes = i8x16::load(src);
    src += stride;
    return shuffle(bytes, byte_order);
}

[[nodiscard]] tt_force_inline void store_audio_samples(
    i8x26 samples,
    std::byte const *&dst,
    int stride,
    i8x16 byte_order
) noexcept
{
    auto bytes = shuffle(samples, byte_order);
    // _mm_maskmoveu_si128
    bytes.store(dst, ~byte_order);
    dst += stride;
}

[[nodiscard]] tt_force_inline i32x4 load_audio_samples(
    std::byte const *src,
    int num_loads,
    int load_stride,
    i8x16 load_byte_order,
    int load_shift
) noexcept
{
    auto r = load_audio_samples(src, load_stride, load_byte_order);

    if (--num_loads) {
        r = byte_shift_left(r, load_shift);
        r ^= load_audio_samples(src, load_stride, load_byte_order);

        if (--num_loads) {
            r = byte_shift_left(r, load_shift);
            r ^= load_audio_samples(src, load_stride, load_byte_order);
            r = byte_shift_left(r, load_shift);
            r ^= load_audio_samples(src, load_stride, load_byte_order);
         }
    }
    return r;
}



}

