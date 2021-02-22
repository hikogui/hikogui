// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string>
#include <ostream>
#include "../strings.hpp"

namespace tt {

/** An IETF BCP 47 Language tag.
 */
class language_tag {
public:
    language_tag(language_tag const &other) noexcept = default;
    language_tag(language_tag &&other) noexcept = default;
    language_tag &operator=(language_tag const &other) noexcept = default;
    language_tag &operator=(language_tag &&other) noexcept = default;

    language_tag() noexcept : tag() {}
    explicit language_tag(std::string tag) noexcept : tag(std::move(tag)) {}
    explicit language_tag(std::string_view tag) noexcept : tag(tag) {}
    explicit language_tag(char const *tag) noexcept : tag(tag) {}

    [[nodiscard]] size_t hash() const noexcept
    {
        return std::hash<std::string>{}(tag);
    }

    explicit operator bool() const noexcept
    {
        return size(tag) != 0;
    }

    [[nodiscard]] language_tag short_tag() const noexcept
    {
        return language_tag{split(tag, '-').front()};
    }

    [[nodiscard]] bool operator==(language_tag const &rhs) const noexcept
    {
        return tag == rhs.tag;
    }

    [[nodiscard]] bool operator!=(language_tag const &rhs) const noexcept = default;

    [[nodiscard]] friend std::string to_string(language_tag const &url) noexcept
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


} // namespace tt

namespace std {

template<>
class hash<tt::language_tag> {
public:
    [[nodiscard]] size_t operator()(tt::language_tag const &rhs) const noexcept
    {
        return rhs.hash();
    }
};

} // namespace std