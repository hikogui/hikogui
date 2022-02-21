// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <ostream>
#include "../strings.hpp"

namespace tt::inline v1 {

/** An IETF BCP 47 Language tag.
 */
class language_tag {
public:
    constexpr language_tag(language_tag const &other) noexcept = default;
    constexpr language_tag(language_tag &&other) noexcept = default;
    constexpr language_tag &operator=(language_tag const &other) noexcept = default;
    constexpr language_tag &operator=(language_tag &&other) noexcept = default;

    constexpr language_tag() noexcept : tag() {}
    constexpr explicit language_tag(std::string tag) noexcept : tag(std::move(tag)) {}
    constexpr explicit language_tag(std::string_view tag) noexcept : tag(tag) {}
    constexpr explicit language_tag(char const *tag) noexcept : tag(tag) {}

    [[nodiscard]] std::size_t hash() const noexcept
    {
        return std::hash<std::string>{}(tag);
    }

    constexpr explicit operator bool() const noexcept
    {
        return size(tag) != 0;
    }

    [[nodiscard]] language_tag short_tag() const noexcept
    {
        return language_tag{split(tag, '-').front()};
    }

    [[nodiscard]] friend constexpr bool operator==(language_tag const &, language_tag const &) noexcept = default;

    [[nodiscard]] friend constexpr std::string to_string(language_tag const &url) noexcept
    {
        return url.tag;
    }

    friend std::ostream &operator<<(std::ostream &lhs, language_tag const &rhs)
    {
        return lhs << to_string(rhs);
    }

private:
    std::string tag;
};

} // namespace tt::inline v1

template<>
class std::hash<tt::language_tag> {
public:
    [[nodiscard]] std::size_t operator()(tt::language_tag const &rhs) const noexcept
    {
        return rhs.hash();
    }
};

template<typename CharT>
struct std::formatter<tt::language_tag, CharT> : std::formatter<std::string_view, CharT> {
    auto format(tt::language_tag const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};
