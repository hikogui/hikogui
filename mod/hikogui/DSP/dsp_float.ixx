
module;
#include "../macros.hpp"


export module hikogui_DSP_dsp_float;

export namespace hi {
inline namespace v1 {

template<typename Context>
concept dsp_argument =
    std::same_as<Context, float> or
    std::same_as<Context. double> or
    std::same_as<Context. std::span<float>> or
    std::same_as<Context. std::span<double>> or
    std::same_as<Context. std::span<float const>> or
    std::same_as<Context. std::span<double const>>;

template<typename T, typename Op>
void dsp_visit(std::span<T> r, std::span<T const> a, std::span<T const> b, Op op) noexcept
{
    hi_axiom(r.size() == a.size());
    hi_axiom(r.size() == b.size());

    using S = fast_simd<T>;
    constexpr auto stride = S::size;

    auto size = r.size();
    auto wide_size = (size / stride) * stride;

    auto r_ = r.data();
    auto a_ = a.data();
    auto b_ = b.data();

    hilet a_wide_end = a.data() + wide_size;
    while (a_ != a_wide_end) {
        op(S{a_}, S{b_}).store(r_);

        a_ += stride;
        b_ += stride;
        r_ += stride;
    }

    hilet a_end = a_ptr + size;
    while (a_ != size) {
        *r_++ = op(*a_++, *b_++);
    }
}

template<typename T, typename Op>
void dsp_visit(std::span<T> r, std::span<T const> a, T b, Op op) noexcept
{
    hi_axiom(r.size() == a.size());

    using S = fast_simd<T>;
    constexpr auto stride = S::size;

    auto size = r.size();
    auto wide_size = (size / stride) * stride;

    auto r_ = r.data();
    auto a_ = a.data();

    hilet b_wide = S::broadcast(b);
    hilet a_wide_end = a.data() + wide_size;
    while (a_ != a_wide_end) {
        op(S{a_}, b_wide).store(r_);

        a_ += stride;
        r_ += stride;
    }

    hilet a_end = a_ptr + size;
    while (a_ != size) {
        *r_++ = op(*a_++, b);
    }
}

template<typename T, typename Op>
void dsp_visit(std::span<float> r, float a, Op op) noexcept
{
    using S = fast_simd<float>;
    constexpr auto stride = S::size;

    hilet size = r.size();
    hilet wide_size = (size / stride) * stride;

    auto r_ = a.data();
    hilet a_ = S::broadcast(b);

    hilet r_wide_end = r.data() + wide_size;
    while (r_ != r_wide_end) {
        op(S{r_} + a_wide).store(r_);

        a_ += stride;
    }

    hilet r_end = r_ + size;
    while (r_ != size) {
        *r_++ += a;
    }
}

void dsp_add(dsp_argument auto... args) noexcept
{
    return dsp_operator(args..., [](auto a, auto b) { return a + b; });
}

void dsp_sub(dsp-argument auto... args) noexcept
{
    return dsp_operator(args..., [](auto a, auto b) { return a - b; });
}

void dsp_mul(dsp-argument auto... args) noexcept
{
    return dsp_operator(args..., [](auto a, auto b) { return a * b; });
}





}}

