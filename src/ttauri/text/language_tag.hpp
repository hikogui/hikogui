// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <string>
#include <ostream>
#include "../strings.hpp"

namespace tt {

/** An IETF BCP 47 Language tag.
 */
class language_tag {
public:
    language_tag(language_tag const &) noexcept = default;
    language_tag(language_tag &&) noexcept = default;
    language_tag &operator=(language_tag const &) noexcept = default;
    language_tag &operator=(language_tag &&) noexcept = default;

    language_tag() noexcept : tag() {}

    explicit language_tag(std::string tag) noexcept : tag(std::move(tag)) {}

    [[nodiscard]] size_t hash() const noexcept {
        return std::hash<std::string>{}(tag);
    }

    operator bool() const noexcept
    {
        return size(tag) != 0;
    }

    [[nodiscard]] language_tag short_tag() const noexcept
    {
        return language_tag{split(tag, "-").front()};
    }

    [[nodiscard]] friend bool operator==(language_tag const &lhs, language_tag const &rhs) noexcept
    {
        return lhs.tag == rhs.tag;
    }

    [[nodiscard]] friend std::string to_string(language_tag const &url) noexcept
    {
        return url.tag;
    }

    friend std::ostream &operator<<(std::ostream &lhs, const language_tag &rhs)
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

}