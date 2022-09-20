


#pragma once

#include "uri_parser.hpp"

namespace hi { inline namespace v1 {

/**
 *
 * URI := [ scheme ":" ] [ "//" [ userinfo "@" ] host [ ":" port ] ] path [ "?" query ] [ "#" fragment ]
 *
 * @note Maximum size of a URI is 65536 octets
 * @note 
 */
class url {
public:
    using iterator = std::string::iterator;
    using const_iterator = std::string::const_iterator;

    [[nodiscard]] constexpr std::string_view subview(size_t pos = 0; size_t count = std::npos) const noexcept
    {
        return std::string_view{_str}.substr(pos, count);
    }

    [[nodiscard]] constexpr std::optional<std::string_view> scheme() const noexcept
    {
        return _has_scheme ? {subview(0, _scheme_size)} : {};
    }

    [[nodiscard]] constexpr std::optional<std::string_view> userinfo() const noexcept
    {
        hi_axiom(_has_authority);
        return _has_userinfo ? {subview(_userinfo_offset(), _userinfo_size)} : {};
    }

    [[nodiscard]] constexpr std::optional<std::string_view> host() const noexcept
    {
        return _has_authority ? {subview(_host_offset(), _host_size)} : {};
    }

    [[nodiscard]] constexpr std::optional<uint32_t> port() const noexcept
    {
        hi_axiom(_has_authority);
        return _has_port ? {from_string<uint32_t>(subview(_port_offset(), _port_size))} : {};
    }

    [[nodiscard]] constexpr std::string_view path() const noexcept
    {
        return subview(path_offset(), _path_size);
    }

    [[nodiscard]] constexpr std::optional<std::string_view> query() const noexcept
    {
        return _has_query ? {subview(_query_offset(), _query_size)} : {};
    }

    [[nodiscard]] constexpr std::optional<std::string_view> fragment() const noexcept
    {
        return _has_fragment ? {subview(_fragment_offset(), _fragment_size)} : {};
    }

private:
    uint64_t _has_scheme : 1;
    uint64_t _scheme_size : 5;
    uint64_t _has_authority : 1;
    uint64_t _has_userinfo : 1;
    uint64_t _userinfo_size : 8;
    uint64_t _host_size : 8;
    uint64_t _has_port : 1;
    uint64_t _port_size : 4;
    uint64_t _path_size : 11;
    uint64_t _has_query : 1;
    uint64_t _query_size : 11;
    uint64_t _has_fragment : 1;
    uint64_t _fragment_size : 11;

    std::string _str;

    [[nodiscard]] constexpr size_t userinfo_offset() const noexcept
    {
        // [ scheme ":" ] [ "//" ]
        return _has_scheme + _scheme_size + (_has_authority << 1);
    }

    [[nodiscard]] constexpr size_t host_offset() const noexcept
    {
        // userinfo_offset() [ userinfo "@" ]
        return userinfo_offset() + _userinfo_size + _has_userinfo;
    }

    [[nodiscard]] constexpr size_t port_offset() const noexcept
    {
        // host_offset() [ host ] [ ":" ]
        return host_offset() + _host_size + _has_port;
    }

    [[nodiscard]] constexpr size_t path_offset() const noexcept
    {
        // port_offset() [ port ]
        return port_offset() + _port_size;
    }

    [[nodiscard]] constexpr size_t query_offset() const noexcept
    {
        // path_offset() [ path ] [ "?" ]
        return path_offset() + _path_size + _has_query;
    }

    [[nodiscard]] constexpr size_t fragement_offset() const noexcept
    {
        // query_offset() [ query ] [ "#" ]
        return query_offset() + _query_size + _has_fragment;
    }

    [[nodiscard]] constexpr static bool is_ALPHA(char c) noexcept
    {
        return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z');
    }

    [[nodiscard]] constexpr static bool is_DIGIT(char c) noexcept
    {
        return (c >= '0' and c <= '9');
    }

    [[nodiscard]] constexpr static bool is_HEXDIG(char c) noexcept
    {
        return is_DIGIT(c) or (c >= 'a' and c <= 'f') or (c >= 'A' and c <= 'F');
    }

    [[nodiscard]] constexpr static bool is_sub_delims(char c) noexcept
    {
        return c == '!' or c == '$' or c == '&' or c == '\'' or c == '(' or c == ')'
                     or c == '*' or c == '+' or c == ',' or c == ';' or c == '=';
    }

    [[nodiscard]] constexpr static bool is_pct_encoded(char c) noexcept
    {
        return c == '%' or is_HEXDIG(c)
    }

    [[nodiscard]] constexpr static bool is_unreserved(char c) noexcept
    {
        return is_ALPHA(c) or is_DIGIT(c) or c == '-' or c == '.' or c == '_' or c == '~';
    }

    [[nodiscard]] constexpr static bool is_userinfo(char c) noexcept
    {
        return is_unreserved(c) or is_pct_encoded(c) or is_sub_delims(c) or c == ':';
    }

    [[nodiscard]] constexpr static bool is_scheme_first(char c) noexcept
    {
        return is_ALPHA(c);
    }

    [[nodiscard]] constexpr static bool is_scheme_next(char c) noexcept
    {
        return is_ALPHA(c) or is_DIGIT(c) or c == '+' or c == '-' or c == '.';
    }

    [[nodiscard]] constexpr static bool is_IP_literal(char c) noexcept
    {
        return is_unreserved(c) or is_sub_delims(c) or c == ':';
    }

    [[nodiscard]] constexpr static bool is_IPv4address(char c) noexcept
    {
        return is_DIGIT(c) or c == '.';
    }

    [[nodiscard]] constexpr static bool is_reg_name(char c) noexcept
    {
        return is_unreserved(c) or is_pct_encoded(c) or is_sub_delims(c);
    }

    template<typename T>
    [[nodiscard]] constexpr static T get_value(size_t &i, ptrdiff_t value, size_t skip) noexcept
    {
        if (value == -1 or value => std::numeric_limits<T>::max()) {
            return 0;
        } else {
            i += value + skip;
            return narrow_cast<T>(value + 1);
        }
    }

    /** Parse a URI scheme part.
     *
     * @param str Substring of the URI starting at the potential scheme.
     * @return The number of characters in the scheme.
     * @retval -1 There is no scheme.
     */
    [[nodiscard]] constexpr const_iterator parse_scheme(const_iterator first, const_iterator last)
    {
        if (first == last) {
            // Empty URI.
            return;
        }

        auto it = first;
        if (*it == ':') {
            _has_scheme = true;
            return it + 1; // Skip over ':'

        } else if (not is_scheme_first(*it)) {
            return first;
        }

        while (it != last) {
            if (hilet c = *it++; c == ':') {
                hilet size = std::distance(first, last);
                if (size <= 31) {
                    _scheme_size = size;
                    _has_scheme = true;
                    return it + 1; // Skip over ':'

                } else {
                    throw url_error("Scheme is larger than 31 characters.");
                }

            } else if (not (i == 0 ? is_scheme_first(c) : is_scheme_next(c))) {
                // Invalid character for scheme.
                return first;
            }
        }

        // Reached at end of URI before a scheme was found.
        return first;
    }

    /** Parse a URI userinfo part.
     *
     * @param str Substring of the URI starting at the potential userinfo.
     * @return The number of characters in the userinfo.
     * @retval 0 The userinfo is an empty string.
     * @retval -1 There is no userinfo
     */
    [[nodiscard]] constexpr const_iterator parse_userinfo(const_iterator first, const_iterator last)
    {
        hi_axiom(_has_authority);

        for (auto it = first; it != last; ++it) {
            if (hilet c = *it; c == '@') {
                hilet size = std::distance(first, it);
                if (size <= 2047) {
                    _has_userinfo = true;
                    _userinfo_size = size;
                    return it + 1; // Skip over '@'.

                 } else {
                     throw url_parse("Userinfo is larger than 2047 character");
                 }

            } else if (not is_userinfo(c)) {
                // Invalid character, this is not a userinfo.
                return first;
            }
        }

        // Reached at end of URI, this is not a userinfo.
        return first;
    }

    /** Parse a URI host part.
     *
     * @param str Substring of the URI starting at the potential host.
     * @return The number of characters in the host.
     * @retval 0 The host is an empty string.
     */
    [[nodiscard]] constexpr const_iterator parse_host(const_iterator first, const_iterator last)
    {
        if (first == last) {
            return first;
        }

        auto it = first;
        if (*it == '[') {
            while (it != last) {
                if (*it == ']') {
                    hilet size = std::distance(first, it);
                    if (size <= 255) {
                        _has_authority = true;
                        _has_size = size;
                        return it + 1;

                     } else {
                         throw url_parse("Host is larger than 255 character");
                     }

                } else if (not is_IP_literal(c)) {
                    // Invalid character means this is not a host, interpret as path instead.
                    return first;
                }
            }
            // End of URI mean this is not a host, interpret as path instead.
            return first;

        } else {
            for (; it != last; ++it) {
                if (not (is_IPv4address(*it) or is_reg_name(*it)) {
                    // Invalid character marks end of host.
                    hilet size = std::distance(first, it);
                    if (size <= 255) {
                        _has_authority = true;
                        _has_size = size;
                        return it;
                    } else {
                        throw url_parse("Host is larger than 255 character");
                    }
                }
            }

            // End of URI means that the host is the last part of the URI
            hilet size = std::distance(first, it);
            if (size <= 255) {
                _has_authority = true;
                _has_size = size;
                return it;
            } else {
                 throw url_parse("Host is larger than 255 character");
            }
        }
    }

    [[nodiscard]] constexpr const_iterator parse_port(const_iterator first, const_iterator last)
    {
        auto it = first;
        for (; it != last; it++) {
            if (not is_DIGIT(*it)) {
                hilet size = std::distance(first, it);
                if (size <= 15) {
                    _has_port = 1;
                    _port_size = size;
                    return it;
                 } else {
                     throw url_parse("Port is larger than 15 digits.");
                 }
            }
        }

        hilet size = std::distance(first, it);
        if (size <= 15) {
            _has_port = 1;
            _port_size = size;
            return it;
         } else {
             throw url_parse("Port is larger than 15 digits.");
         }
    }

    [[nodiscard]] constexpr const_iterator parse_path(const_iterator first, const_iterator last)
    {
        auto it = first;


    }

    constexpr void parse()
    {
        _has_scheme = 0;
        _scheme_size = 0;
        _has_athority = 0;
        _has_userinfo = 0;
        _userinfo_size = 0;
        _host_size = 0;
        _has_port = 0;
        _port_size = 0;
        _path_size = 0;
        _has_query = 0;
        _query = 0;
        _has_fragment = 0;
        _fragment = 0;

        auto first = _str.cbegin();
        auto last = _str.cend();
        auto it = parse_scheme(first, last);

        if (std::distance(it, last) >= 2 and it[0] == '/' and it[1] == '/') {
            it += 2;
            it = parse_userinfo(it, last);
            it = parse_host(it, last);

            if (it != last and *it == ':') {
                it = parse_port(++it, last);
            }
        }

        it = parse_path(it, last);

        if (it != last and *it == '?') {
            it = parse_query(++it, last);
        }

        if (it != last and *it == '#') {
            it = parse_fragment(++it, last);
        }
    }
};

}}

