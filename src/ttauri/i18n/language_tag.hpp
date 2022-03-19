// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "iso639.hpp"
#include "iso15924.hpp"
#include "iso3166.hpp"
#include "../check.hpp"
#include "../strings.hpp"

namespace tt::inline v1 {

class language_tag {
public:
    constexpr language_tag() noexcept {}
    constexpr language_tag(language_tag const &) noexcept = default;
    constexpr language_tag(language_tag &&) noexcept = default;
    constexpr language_tag &operator=(language_tag const &) noexcept = default;
    constexpr language_tag &operator=(language_tag &&) noexcept = default;

    constexpr language_tag(iso639 const &language, iso15924 const &script = {}, iso3166 const &region = {}) noexcept :
        _language(language), _script(script), _region(region)
    {
    }

    constexpr language_tag(iso639 const &language, iso3166 const &region) noexcept : language_tag(language, {}, region) {}

    language_tag(std::string_view str) : _language(), _script(), _region()
    {
        for (ttlet element : std::views::split(str, std::string_view{"-"})) {
            if (not _language) {
                // 2 or 3 letter language code.
                _language = iso639{element};

            } else if (element.size() == 4 and not _script and not _region) {
                // 4 letter script code.
                _script = iso15924{element};

            } else if (element.size() == 3 and not _region) {
                // 3 digit region code.
                tt_not_implemented();

            } else if (element.size() == 2 and not _region) {
                // 2 letter region code.
                _region = iso3166{element};

            } else {
                // Ignore stuff on the end.
                ;
            }
        }
    }

    [[nodiscard]] constexpr static language_tag language_only(language_tag const &other) const noexcept
    {
        return language_tag{other._language};
    }

    [[nodiscard]] constexpr iso639 language() const noexcept
    {
        return _language;
    }

    [[nodiscard]] iso15924 script() const noexcept
    {
        if (_script) {
            return _script;
        } else {
            return _language.default_script();
        }
    }

    [[nodiscard]] std::string to_string() const noexcept
    {
        auto r = std::string{};
        r += static_cast<std::string>(_language);
        if (_script) {
            r += '-';
            r += static_cast<std::string>(_script);
        }
        if (_region) {
            r += "-";
            r += static_cast<std::string>(_region);
        }
        return r;
    }

private:
    iso639 _language;
    iso15924 _script;
    iso3166 _region;
};

} // namespace tt::inline v1

template<typename CharT>
struct std::formatter<tt::language_tag, CharT> : std::formatter<std::string_view, CharT> {
    auto format(tt::language_tag const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};
