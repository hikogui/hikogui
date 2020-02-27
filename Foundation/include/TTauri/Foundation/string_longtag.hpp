// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/hash.hpp"
#include <string>
#include <string_view>

namespace TTauri {

[[nodiscard]] constexpr uint64_t char_to_longtag(char c, bool convert=false) noexcept {
    switch (c) {
    case '\0': return 0;
    case '.': return 37;
    case '-': return 38;
    case '_': return 39;
    default:
        if (c >= 'a' && c <= 'z') { // 1:26.
            return (c - 'a') + 1;
        } else if (c >= 'A' && c <= 'Z') {
            return (c - 'A') + 1;
        } else if (c >= '0' && c <= '9') {
            return (c - '0') + 27; // 27:36
        } else if (convert) {
            return 39; // Convert to underscore.
        } else {
            no_default;
        }
    }
}

[[nodiscard]] constexpr char longtag_to_char(uint64_t v) noexcept {
    switch (v) {
    case 0: return '\0';
    case 37: return '.';
    case 38: return '-';
    case 39: return '_';
    default:
        if (v >= 1 && v <= 26) {
            return (static_cast<char>(v) - 1) + 'a';
        } else if (v >= 27 && v <= 36) {
            return (static_cast<char>(v) - 27) + '0';
        } else {
            no_default;
        }
    }
}

/** Convert a c-string to a 128-bit integer.
 * This function uses a 39 character alphabet (excluding nul).
 * @param convert If true the function will convert unknown characters to '_' and truncate overlong strings.
 *                When false unknown characters and overlong strings will cause the application to terminate.
 * @return high, low: With the first character aligned to the most significant bits of high.
 */
[[nodiscard]] constexpr std::pair<uint64_t,uint64_t> string_to_longtag(const char *s, bool convert=false) noexcept {
    int8_t i = 0;
    uint64_t high = 0;
    uint64_t low = 0;
    bool eof = false;

    for (i = 0; i != 12; ++i) {
        high *= 40;

        if (eof || (s[i] == '\0')) {
            eof = true;
        } else {
            high += char_to_longtag(s[i], convert);
        }
    }
    for (; i != 24; ++i) {
        low *= 40;

        if (eof || (s[i] == '\0')) {
            eof = true;
        } else {
            low += char_to_longtag(s[i], convert);
        }
    }

    if (eof || (s[i] == '\0')) {
        eof = true;
    }

    if (eof || convert) {
        return {high, low};
    } else {
        no_default;
    }
}

/** Convert a 128-bit integer to a c-string.
* This function uses a 39 character alphabet (excluding nul).
*/
constexpr void longtag_to_string(std::array<char,25> s, uint64_t high, uint64_t low)
{
    // Terminate the string in case the string is the full 24 characters in length.
    s[24] = '\0';
    for (int8_t i = 23; i >= 12; --i) {
        s[i] = longtag_to_char(low % 40);
        low /= 40;
    }
    for (int8_t i = 11; i >= 0; --i) {
        s[i] = longtag_to_char(high % 40);
        high /= 40;
    }
}

class string_longtag {
    uint64_t high;
    uint64_t low;

public:
    constexpr string_longtag(string_longtag const &other) noexcept = default;
    constexpr string_longtag(string_longtag &&other) noexcept = default;
    constexpr string_longtag &operator=(string_longtag const &other) noexcept = default;
    constexpr string_longtag &operator=(string_longtag &&other) noexcept = default;

    constexpr string_longtag() noexcept :
        high(0), low(0) {}

    constexpr string_longtag(char const *s) noexcept :
        high(0), low(0)
    {
        let &p = string_to_longtag(s);
        high = p.first;
        low = p.second;
    }

    string_longtag(std::string const &s) noexcept :
        string_longtag(s.data()) {}

    string_longtag(std::string_view const &s) noexcept :
        string_longtag(s.data()) {}

    constexpr string_longtag &operator=(char const *s) noexcept {
        let &p = string_to_longtag(s);
        high = p.first;
        low = p.second;
        return *this;
    }

    string_longtag &operator=(std::string const &s) noexcept {
        return (*this = s.data());
    }

    string_longtag &operator=(std::string_view const &s) noexcept {
        return (*this = s.data());
    }

    explicit operator std::string () const noexcept {
        std::array<char,25> buffer;
        longtag_to_string(buffer, high, low);
        return {buffer.data()};
    }

    size_t hash() const noexcept {
        return hash_mix_two(std::hash<uint64_t>{}(high), std::hash<uint64_t>{}(low));
    }

    [[nodiscard]] constexpr friend bool operator==(string_longtag const &lhs, string_longtag const &rhs) noexcept {
        return lhs.high == rhs.high && lhs.low == rhs.low;
    }

    [[nodiscard]] constexpr friend bool operator<(string_longtag const &lhs, string_longtag const &rhs) noexcept {
        if (lhs.high == rhs.high) {
            return lhs.low < rhs.low;
        } else {
            return lhs.high < rhs.high;
        }
    }

    [[nodiscard]] constexpr friend bool operator!=(string_longtag const &lhs, string_longtag const &rhs) noexcept { return !(lhs == rhs); }
    [[nodiscard]] constexpr friend bool operator>(string_longtag const &lhs, string_longtag const &rhs) noexcept { return rhs < lhs; }
    [[nodiscard]] constexpr friend bool operator<=(string_longtag const &lhs, string_longtag const &rhs) noexcept { return !(lhs > rhs); }
    [[nodiscard]] constexpr friend bool operator>=(string_longtag const &lhs, string_longtag const &rhs) noexcept { return !(lhs < rhs); }

    [[nodiscard]] friend std::string to_string(string_longtag const &rhs) noexcept {
        return static_cast<std::string>(rhs);
    }

    friend std::ostream &operator<<(std::ostream &lhs, string_longtag const &rhs) {
        return lhs << to_string(rhs);
    }

};

}

namespace std {

template<>
struct hash<TTauri::string_longtag> {
    [[nodiscard]] size_t operator() (TTauri::string_longtag const &rhs) const noexcept {
        return rhs.hash();
    }
};

}
