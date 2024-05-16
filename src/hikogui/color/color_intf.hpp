// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file color/color.hpp Defined the color type.
 * @ingroup color
 */

#pragma once

#include "../geometry/geometry.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <hikocpu/hikocpu.hpp>
#include <string>
#include <map>
#include <mutex>
#include <format>

hi_export_module(hikogui.color : color_intf);

hi_export namespace hi {
inline namespace v1 {

/** This is a RGBA floating point color.
 * The color can be converted between different color spaces using the matrix-class.
 *
 * But in most cases in the application and hikogui library this color would be in
 * the scRGBA color space. This color space is compatible with the sRGB standard
 * IEC 61966-2-1:1999.
 *
 * scRGB details:
 * - the ITU-R BT.709 color primaries.
 * - A linear transfer function (unlike sRGB).
 * - R=0.0, G=0.0, B=0.0: Black
 * - R=1.0, G=1.0, B=1.0: White D65 at 80 nits (80 cd/m^2).
 * - RGB values above 1.0 are allowed for HDR (high dynamic range)
 * - RGB values below 0.0 are allowed for WCG (wide color gamut)
 *
 * scRGBA details:
 * - Includes an alpha value
 * - Alpha values are linear and must be between 0.0 and 1.0.
 * - A=0.0 fully transparent
 * - A=1.0 fully opaque
 * - RGB values are NOT pre-multiplied with the alpha.
 *
 * @ingroup color
 */
class color {
public:
    constexpr color() noexcept = default;
    constexpr color(color const&) noexcept = default;
    constexpr color(color&&) noexcept = default;
    constexpr color& operator=(color const&) noexcept = default;
    constexpr color& operator=(color&&) noexcept = default;

    [[nodiscard]] constexpr explicit color(f16x4 const& other) noexcept : _v(other)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit color(f32x4 const& other) noexcept : color(static_cast<f16x4>(other)) {}

    [[nodiscard]] constexpr explicit operator f16x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return static_cast<f32x4>(_v);
    }

    [[nodiscard]] constexpr color(float r, float g, float b, float a = 1.0f) noexcept : color(f32x4{r, g, b, a}) {}

    /** List color names.
     */
    [[nodiscard]] static inline std::vector<std::string> list() noexcept;

    /** Find a color by name.
     *
     * @param name The name of the color to find.
     * @return A pointer to a writable named-color. Or nullptr when not found.
     */
    [[nodiscard]] static inline color* find(std::string const& name) noexcept;

    [[nodiscard]] static inline color black() noexcept;
    [[nodiscard]] static inline color silver() noexcept;
    [[nodiscard]] static inline color gray() noexcept;
    [[nodiscard]] static inline color white() noexcept;
    [[nodiscard]] static inline color maroon() noexcept;
    [[nodiscard]] static inline color red() noexcept;
    [[nodiscard]] static inline color purple() noexcept;
    [[nodiscard]] static inline color fuchsia() noexcept;
    [[nodiscard]] static inline color green() noexcept;
    [[nodiscard]] static inline color lime() noexcept;
    [[nodiscard]] static inline color olive() noexcept;
    [[nodiscard]] static inline color yellow() noexcept;
    [[nodiscard]] static inline color navy() noexcept;
    [[nodiscard]] static inline color blue() noexcept;
    [[nodiscard]] static inline color teal() noexcept;
    [[nodiscard]] static inline color aqua() noexcept;
    [[nodiscard]] static inline color indigo() noexcept;
    [[nodiscard]] static inline color orange() noexcept;
    [[nodiscard]] static inline color pink() noexcept;
    [[nodiscard]] static inline color gray0() noexcept;
    [[nodiscard]] static inline color gray1() noexcept;
    [[nodiscard]] static inline color gray2() noexcept;
    [[nodiscard]] static inline color gray3() noexcept;
    [[nodiscard]] static inline color gray4() noexcept;
    [[nodiscard]] static inline color gray5() noexcept;
    [[nodiscard]] static inline color gray6() noexcept;
    [[nodiscard]] static inline color gray7() noexcept;
    [[nodiscard]] static inline color gray8() noexcept;
    [[nodiscard]] static inline color gray9() noexcept;
    [[nodiscard]] static inline color gray10() noexcept;
    [[nodiscard]] static inline color transparent() noexcept;

    [[nodiscard]] size_t hash() const noexcept
    {
        return std::hash<uint64_t>{}(std::bit_cast<uint64_t>(_v));
    }

    [[nodiscard]] constexpr half& r() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr half& g() noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr half& b() noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr half& a() noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr half r() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr half g() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr half b() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr half a() const noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() >= 0.0 && _v.w() <= 1.0;
    }

    [[nodiscard]] constexpr friend bool operator==(color const& lhs, color const& rhs) noexcept
    {
        return equal(lhs._v, rhs._v);
    }

    [[nodiscard]] constexpr friend color operator*(color const& lhs, color const& rhs) noexcept
    {
        return color{lhs._v * rhs._v};
    }

    /** Transform a color by a color matrix.
     *
     * The alpha value is not included in the transformation and copied from the input.
     *
     * @note It is undefined behavior if the matrix contains a translation.
     * @param lhs The 3x3 color transformation matrix to use.
     * @param rhs The color to be transformed.
     * @return The transformed color.
     */
    [[nodiscard]] constexpr friend color operator*(matrix3 const& lhs, color const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());

        auto const rhs_ = static_cast<f32x4>(rhs);

        auto r = color{get<0>(lhs) * rhs_.xxxx() + get<1>(lhs) * rhs_.yyyy() + get<2>(lhs) * rhs_.zzzz() + get<3>(lhs)};

        r.a() = rhs.a();
        return r;
    }

    friend std::ostream& operator<<(std::ostream& lhs, color const& rhs)
    {
        return lhs << std::format("rgb({} {} {} {})", rhs.r(), rhs.g(), rhs.b(), rhs.a());
    }

private:
    f16x4 _v = {};
};

namespace detail {

class named_color_base {
public:
    named_color_base(std::string name, hi::color color) noexcept : _color(color)
    {
        auto const lock = std::scoped_lock(_map_mutex);
        _map[name] = this;
    }

    named_color_base(named_color_base const&) = delete;
    named_color_base(named_color_base&&) = delete;
    named_color_base& operator=(named_color_base const&) = delete;
    named_color_base& operator=(named_color_base&&) = delete;

    hi::color const& operator*() const noexcept
    {
        return _color;
    }

    hi::color& operator*() noexcept
    {
        return _color;
    }

    operator hi::color() const noexcept
    {
        return _color;
    }

    [[nodiscard]] static std::vector<std::string> list() noexcept
    {
        auto r = std::vector<std::string>{};

        auto const lock = std::scoped_lock(_map_mutex);
        for (auto& item : _map) {
            r.push_back(item.first);
        }

        return r;
    }

    [[nodiscard]] static named_color_base* find(std::string const& name) noexcept
    {
        auto const lock = std::scoped_lock(_map_mutex);
        auto const it = _map.find(name);
        if (it != _map.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }

private:
    // The map is protected with a mutex because global variable initialization
    // may be deferred and run on a different threads. However we can not
    // use the deadlock detector as it will use a thread_local variable.
    // The initialization order of static global variables and thread_local
    // variables are undetermined.
    inline static std::map<std::string, named_color_base*> _map;
    inline static std::mutex _map_mutex;

protected:
    color _color;
};

template<fixed_string Tag>
class named_color_type : public named_color_base {
public:
    named_color_type() noexcept : named_color_base(Tag, hi::color{}) {}
    named_color_type(hi::color color) noexcept : named_color_base(Tag, color) {}

    named_color_type& operator=(hi::color color) noexcept
    {
        _color = color;
        return *this;
    }
};

} // namespace detail

template<fixed_string Tag>
inline auto named_color = detail::named_color_type<Tag>{};

[[nodiscard]] inline std::vector<std::string> color::list() noexcept
{
    return detail::named_color_base::list();
}

[[nodiscard]] inline color* color::find(std::string const& name) noexcept
{
    if (auto named_color_ptr = detail::named_color_base::find(name)) {
        return std::addressof(**named_color_ptr);
    } else {
        return nullptr;
    }
}

// clang-format off

template<> inline auto named_color<"black"> = detail::named_color_type<"black">{color{0, 0, 0}};
template<> inline auto named_color<"silver"> = detail::named_color_type<"silver">{color{192, 192, 192}};
template<> inline auto named_color<"gray"> = detail::named_color_type<"gray">{color{128, 128, 128}};
template<> inline auto named_color<"white"> = detail::named_color_type<"white">{color{255, 255, 255}};
template<> inline auto named_color<"maroon"> = detail::named_color_type<"maroon">{color{128, 0, 0}};
template<> inline auto named_color<"red"> = detail::named_color_type<"red">{color{255, 0, 0}};
template<> inline auto named_color<"purple"> = detail::named_color_type<"purple">{color{128, 0, 128}};
template<> inline auto named_color<"fuchsia"> = detail::named_color_type<"fuchsia">{color{255, 0, 255}};
template<> inline auto named_color<"green"> = detail::named_color_type<"green">{color{0, 128, 0}};
template<> inline auto named_color<"lime"> = detail::named_color_type<"lime">{color{0, 255, 0}};
template<> inline auto named_color<"olive"> = detail::named_color_type<"olive">{color{128, 128, 0}};
template<> inline auto named_color<"yellow"> = detail::named_color_type<"yellow">{color{255, 255, 0}};
template<> inline auto named_color<"navy"> = detail::named_color_type<"navy">{color{0, 0, 128}};
template<> inline auto named_color<"blue"> = detail::named_color_type<"blue">{color{0, 0, 255}};
template<> inline auto named_color<"teal"> = detail::named_color_type<"teal">{color{0, 128, 128}};
template<> inline auto named_color<"aqua"> = detail::named_color_type<"aqua">{color{0, 255, 255}};
template<> inline auto named_color<"indigo"> = detail::named_color_type<"indigo">{color{75, 0, 130}};
template<> inline auto named_color<"orange"> = detail::named_color_type<"orange">{color{255, 165, 0}};
template<> inline auto named_color<"pink"> = detail::named_color_type<"pink">{color{255, 192, 203}};
template<> inline auto named_color<"gray0"> = detail::named_color_type<"gray0">{color{0, 0, 0}};
template<> inline auto named_color<"gray1"> = detail::named_color_type<"gray1">{color{26, 26, 26}};
template<> inline auto named_color<"gray2"> = detail::named_color_type<"gray2">{color{51, 51, 51}};
template<> inline auto named_color<"gray3"> = detail::named_color_type<"gray3">{color{77, 77, 77}};
template<> inline auto named_color<"gray4"> = detail::named_color_type<"gray4">{color{102, 102, 102}};
template<> inline auto named_color<"gray5"> = detail::named_color_type<"gray5">{color{127, 127, 127}};
template<> inline auto named_color<"gray6"> = detail::named_color_type<"gray6">{color{153, 153, 153}};
template<> inline auto named_color<"gray7"> = detail::named_color_type<"gray7">{color{179, 179, 179}};
template<> inline auto named_color<"gray8"> = detail::named_color_type<"gray8">{color{204, 204, 204}};
template<> inline auto named_color<"gray9"> = detail::named_color_type<"gray9">{color{229, 229, 229}};
template<> inline auto named_color<"gray10"> = detail::named_color_type<"gray10">{color{255, 255, 255}};
template<> inline auto named_color<"transparent"> = detail::named_color_type<"transparent">{color{0, 0, 0, 0}};

[[nodiscard]] inline color color::black() noexcept { return named_color<"black">; }
[[nodiscard]] inline color color::silver() noexcept { return named_color<"silver">; }
[[nodiscard]] inline color color::gray() noexcept { return named_color<"gray5">; }
[[nodiscard]] inline color color::white() noexcept { return named_color<"white">; }
[[nodiscard]] inline color color::maroon() noexcept { return named_color<"maroon">; }
[[nodiscard]] inline color color::red() noexcept { return named_color<"red">; }
[[nodiscard]] inline color color::purple() noexcept { return named_color<"purple">; }
[[nodiscard]] inline color color::fuchsia() noexcept { return named_color<"fuchsia">; }
[[nodiscard]] inline color color::green() noexcept { return named_color<"green">; }
[[nodiscard]] inline color color::lime() noexcept { return named_color<"lime">; }
[[nodiscard]] inline color color::olive() noexcept { return named_color<"olive">; }
[[nodiscard]] inline color color::yellow() noexcept { return named_color<"yellow">; }
[[nodiscard]] inline color color::navy() noexcept { return named_color<"navy">; }
[[nodiscard]] inline color color::blue() noexcept { return named_color<"blue">; }
[[nodiscard]] inline color color::teal() noexcept { return named_color<"teal">; }
[[nodiscard]] inline color color::aqua() noexcept { return named_color<"aqua">; }
[[nodiscard]] inline color color::indigo() noexcept { return named_color<"indigo">; }
[[nodiscard]] inline color color::orange() noexcept { return named_color<"orange">; }
[[nodiscard]] inline color color::pink() noexcept { return named_color<"pink">; }
[[nodiscard]] inline color color::gray0() noexcept { return named_color<"gray0">; }
[[nodiscard]] inline color color::gray1() noexcept { return named_color<"gray1">; }
[[nodiscard]] inline color color::gray2() noexcept { return named_color<"gray2">; }
[[nodiscard]] inline color color::gray3() noexcept { return named_color<"gray3">; }
[[nodiscard]] inline color color::gray4() noexcept { return named_color<"gray4">; }
[[nodiscard]] inline color color::gray5() noexcept { return named_color<"gray5">; }
[[nodiscard]] inline color color::gray6() noexcept { return named_color<"gray6">; }
[[nodiscard]] inline color color::gray7() noexcept { return named_color<"gray7">; }
[[nodiscard]] inline color color::gray8() noexcept { return named_color<"gray8">; }
[[nodiscard]] inline color color::gray9() noexcept { return named_color<"gray9">; }
[[nodiscard]] inline color color::gray10() noexcept { return named_color<"gray10">; }
[[nodiscard]] inline color color::transparent() noexcept { return named_color<"transparent">; }
// clang-format on

} // namespace v1
} // namespace hi::v1

template<>
struct std::hash<hi::color> {
    [[nodiscard]] size_t operator()(hi::color const& rhs) const noexcept
    {
        return rhs.hash();
    }
};

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::color, char> : std::formatter<std::string, char> {
    auto format(hi::color const& t, auto& fc) const
    {
        return std::formatter<std::string, char>::format(std::format("rgba({}, {}, {}, {})", t.r(), t.g(), t.b(), t.a()), fc);
    }
};
