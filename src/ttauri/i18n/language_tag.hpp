// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "iso_15924.hpp"
#include "iso_3166.hpp"
#include "iso_639.hpp"
#include "../hash.hpp"

namespace tt::inline v1 {

class language_tag {
public:
    iso_639 language;
    iso_15924 script;
    iso_3166 region;

    constexpr language_tag() noexcept {}
    constexpr language_tag(language_tag const&) noexcept = default;
    constexpr language_tag(language_tag&&) noexcept = default;
    constexpr language_tag& operator=(language_tag const&) noexcept = default;
    constexpr language_tag& operator=(language_tag&&) noexcept = default;

    constexpr language_tag(iso_639 const& language, iso_15924 const& script = {}, iso_3166 const& region = {}) noexcept :
        language(language), script(script), region(region)
    {
    }

    constexpr language_tag(iso_639 const& language, iso_3166 const& region) noexcept : language_tag(language, {}, region) {}

    language_tag(std::string_view str);

    [[nodiscard]] bool empty() const noexcept
    {
        return language.empty() and script.empty() and region.empty();
    }

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Get a tag with only the language.
     */
    [[nodiscard]] language_tag short_tag() const noexcept
    {
        return language_tag{language};
    }

    [[nodiscard]] std::string to_string() const noexcept
    {
        auto r = std::string{};
        r += language.code();
        if (script) {
            r += '-';
            r += script.code4();
        }
        if (region) {
            r += "-";
            r += region.code2();
        }
        return r;
    }

    [[nodiscard]] constexpr friend bool operator==(language_tag const&, language_tag const&) noexcept = default;
};

} // namespace tt::inline v1

template<>
struct std::hash<tt::language_tag> {
    [[nodiscard]] size_t operator()(tt::language_tag const& rhs) const noexcept
    {
        return tt::hash_mix(
            std::hash<tt::iso_639>{}(rhs.language),
            std::hash<tt::iso_15924>{}(rhs.script),
            std::hash<tt::iso_3166>{}(rhs.region));
    }
};

template<typename CharT>
struct std::formatter<tt::language_tag, CharT> : std::formatter<std::string_view, CharT> {
    auto format(tt::language_tag const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(t.to_string(), fc);
    }
};
