// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"
#include <string_view>
#include <string>
#include <vector>
#include <functional>

namespace TTauri {

constexpr char native_path_seperator = (OperatingSystem::current == OperatingSystem::Windows) ? '\\' : '/';

constexpr bool is_urlchar_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

constexpr bool is_urlchar_digit(char c) {
    return (c >= '0' && c <= '9');
}

constexpr bool is_urlchar_gen_delims(char c) {
    return c == ':' || c == '/' || c == '?' || c == '#' || c == '[' || c == ']' || c == '@';
}

constexpr bool is_urlchar_sub_delims(char c) {
    return
        c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' ||
        c == '*' || c == '+' || c == ',' || c == ';' || c == '=';
}

constexpr bool is_urlchar_unreserved(char c) {
    return is_urlchar_alpha(c) || is_urlchar_digit(c) || c == '-' || c == '.' || c == '_' || c == '~';
}

constexpr bool is_urlchar_reserved(char c) {
    return is_urlchar_gen_delims(c) || is_urlchar_sub_delims(c);
}

constexpr bool is_urlchar_pchar(char c) {
    return is_urlchar_unreserved(c) || is_urlchar_sub_delims(c) || c == ':' || c == '@';
}

constexpr bool is_urlchar_pchar_forward(char c) {
    return is_urlchar_pchar(c) || c == '/';
}

constexpr bool is_urlchar_pchar_backward(char c) {
    return is_urlchar_pchar(c) || c == '\\';
}

/*! Replace reserved characters with percent-encoding.
 *
 * \param input string to potentially encode.
 * \param unreserved_char_check a function pointer to check if a character is unreserved.
 * \return The url-encoded string.
 */
std::string url_encode_part(std::string_view input, std::function<bool(char)> unreserved_char_check) noexcept;

inline std::string url_encode(std::string_view input) noexcept {
    return url_encode_part(input, is_urlchar_unreserved);
}


/*! Replace all percent-encoding with actual characters from a part of a URL.
 *
 * space-to-plus encoding is part of encoding a query string inside application/x-www-form-urlencoded.
 * It is probably safe do url_decode space-to-plus for query strings.
 *
 * \param input url-encoded string.
 * \param plus_to_space Convert '+' character to space ' ' characters.
 * \return 
 */
std::string url_decode(std::string_view input, bool plus_to_space=false) noexcept;

/*! A URL split into its parts.
 * Each part is url_encoded.
 */
struct url_parts {
    std::string_view scheme;
    std::string_view authority;
    std::string_view drive;
    bool absolute;
    std::vector<std::string_view> segments;
    std::string_view query;
    std::string_view fragment;
};

/*! Parse a url and return its parts.
 *
 * \param url The url to parse.
 * \return The URL parsed into its parts.
 */
url_parts parse_url(std::string_view url) noexcept;

/*! Parse a filesystem path and return its parts.
 * This will detect the path seperator that is used in the path.
 * On windows this is useful because both forward-slash and back-slash is possible.
 *
 * Because parse_path() creates a temporary string that is a url_encoded() version of
 * the path, and url_parts will contain views to this temporary string, we need pass
 * this temporary string as a parameter to handle ownership correctly.
 *
 * \param path A UTF-8 string representing a file.
 * \param encodedPath A string that was created in the scope where url_parts is used.
 * \return The Path parsed into its parts.
 */
url_parts parse_path(std::string_view path, std::string &encodedPath) noexcept;

std::string generate_url(url_parts const &parts) noexcept;

std::string generate_path(url_parts const &parts, char sep='/') noexcept;

std::string generate_native_path(url_parts const &parts) noexcept;

/*! Normalize the parts of an url.
 */
void normalize_url_parts(url_parts &parts) noexcept;

/*! Normalize the parts of an url.
 */
std::string normalize_url(std::string_view url) noexcept;

/*! Concatonate paths.
 */
url_parts concatenate_url_parts(url_parts const &lhs, url_parts const &rhs) noexcept;

/*! Concatonate paths.
*/
std::string concatenate_url(std::string_view const lhs, std::string_view const rhs) noexcept;

/*! Extract a filename from a path.
 */
std::string filename_from_path(std::string_view path) noexcept;

}