// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "cast.hpp"
#include "exception.hpp"
#include "ranges.hpp"
#include "generator.hpp"
#include <string>
#include <optional>
#include <ranges>

namespace hi { inline namespace v1 {

/**
 *
 * @note Maximum size of a URI is 65535 octets
 */
class URI {
public:
    /** Construct a URL from a string.
     *
     * @note This constructor will normalize the URI
     * @throws uri_error When the URI can not be normalized due to a parse error.
     */
    constexpr URI(std::string_view str) : _str(URI(raw{}, str).normalized_string())
    {
        parse();
    }

    /** Get the scheme-component of the URI.
     *
     * @return The optional and lower-cased scheme-component.
     */
    [[nodiscard]] constexpr std::optional<std::string> scheme() const noexcept
    {
        return get_scheme();
    }

    /** Get the userinfo-component of the URI.
     *
     * @return The optional and decoded userinfo-component.
     */
    [[nodiscard]] constexpr std::optional<std::string> userinfo() const noexcept
    {
        return get_userinfo();
    }

    /** Get the host-component of the URI.
     *
     * @return The optional and decoded host-component.
     */
    [[nodiscard]] constexpr std::optional<std::string> host() const noexcept
    {
        return get_host();
    }

    /** Get the port-component of the URI.
     *
     * @return The optional port-component.
     */
    [[nodiscard]] constexpr std::optional<std::string> port() const noexcept
    {
        return get_port();
    }

    /** Check if the path-component is an absolute path.
     *
     * @return true if the path-component is an absolute path, false if the path-component is a relative path.
     */
    [[nodiscard]] constexpr bool path_is_absolute() const noexcept
    {
        return _path_is_absolute;
    }

    /** Check if the path-component is a directory.
     *
     * @note If the path-component is a directory then the last segment is empty.
     * @return true if the path-component is a directory, false if the path-component is a file.
     */
    [[nodiscard]] constexpr bool path_is_directory() const noexcept
    {
        return _path_is_directory;
    }

    /** Get the number of segments of the path.
     *
     * @return The number of segment of the path.
     */
    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _num_segments;
    }

    /** Get the segments of the path.
     *
     * @note path_is_absolute() is needed to determine if there is a leading slash '/'.
     * @return A vector of decoded segments. The last segment is a filename, if the last segment is empty than the path is a
     * directory.
     */
    [[nodiscard]] constexpr std::vector<std::string> segments() const noexcept
    {
        auto r = std::vector<std::string>{};
        r.reserve(size());

        hilet path = raw_path();

        // Skip leading slash.
        auto first = wide_cast<size_t>(path_is_absolute());

        // Skip subsequent slashes of each segment.
        while (first < path.size()) {
            hilet last = path.find('/', first);
            r.push_back(decode(path.substr(first, last)));
            first = last + 1;
        }

        if (_path_is_directory) {
            r.emplace_back();
        }

        return r;
    }

    /** Get a segments of the path.
     *
     * @note path_is_absolute() is needed to determine if there is a leading slash '/'.
     * @note It is undefined behavior to index beyond the number of segments.
     * @note The last segment is a filename, if the last segment is empty than the path is a directory.
     * @param index The index into the segments.
     * @return A decoded segment.
     */
    [[nodiscard]] constexpr std::string operator[](size_t index) const noexcept
    {
        hi_axiom(index < size());
        auto path = raw_path();

        // Skip leading slash.
        auto first = wide_cast<size_t>(path_is_absolute());

        // Skip subsequent slashes of each segment.
        for (; index != 0; --index) {
            first = path.find('/', first) + 1;
        }

        // Find the next slash.
        auto last = path.find('/', first);

        // Get the segment between slashes, or the last segment.
        auto segment = path.substr(first, last);
        return decode(segment);
    }

    /** Get a segments of the path.
     *
     * @note path_is_absolute() is needed to determine if there is a leading slash '/'.
     * @note The last segment is a filename, if the last segment is empty than the path is a directory.
     * @param index The index into the segments.
     * @return A decoded segment.
     * @throws std::out_of_range When the index is beyond the number of segments returned by `size()`.
     */
    [[nodiscard]] constexpr std::string at(size_t index) const
    {
        if (index < size()) {
            return (*this)[index];
        } else {
            throw std::out_of_range("URI segment index out of range.");
        }
    }

    /** Get the query-component of the URI.
     *
     * @return The optional and decoded query-component.
     */
    [[nodiscard]] constexpr std::optional<std::string> query() const noexcept
    {
        return get_query();
    }

    /** Get the fragment-component of the URI.
     *
     * @return The optional and decoded fragment-component.
     */
    [[nodiscard]] constexpr std::optional<std::string> fragment() const noexcept
    {
        return get_fragment();
    }

    /** URI percent-encoding decode function.
     *
     * @param str A percent-encoded string.
     * @return A UTF-8 encoded string.
     */
    [[nodiscard]] constexpr static std::string decode(std::string_view str)
    {
        auto r = std::string{};
        r.reserve(str.size());

        auto state = 2;
        uint8_t code_unit = 0;
        for (auto c : str) {
            switch (state) {
            case 0:
                [[fallthrough]];
            case 1:
                code_unit <<= 4;
                if (c >= '0' and c <= '9') {
                    code_unit |= c & 0xf;
                } else if ((c >= 'a' and c <= 'f') or (c >= 'A' and c <= 'F')) {
                    code_unit |= (c & 0xf) + 9;
                } else {
                    throw uri_error("Unexpected character in percent-encoding.");
                }
                if (++state == 2) {
                    r += char_cast<char>(code_unit);
                }
                break;
            case 2:
                if (c == '%') {
                    state = 0;
                } else {
                    r += c;
                }
                break;
            default:
                hi_no_default();
            }
        }

        if (state != 2) {
            throw uri_error("Unexpected end of URI component inside percent-encoding.");
        }

        return r;
    }

    /** URI encode a component.
     *
     * @tparam extras The extra characters beyond the unreserved characters to pct-encode.
     * @param rhs An UTF-8 encoded string, a component or sub-component of a URI.
     * @return A percent-encoded string.
     */
    template<char... Extras>
    [[nodiscard]] constexpr static std::string encode(std::string_view rhs) noexcept
    {
        auto r = std::string{};
        r.reserve(rhs.size());

        for (auto c : rhs) {
            // clang-format off
            if (((
                    (c >= 'a' and c <= 'z') or
                    (c >= 'A' and c <= 'Z') or
                    (c >= '0' and c <= '9') or
                    c == '-' or c == '.' or c == '_' or c == '~'
                ) or ... or (c == Extras))) {
                r += c;

            } else {
                r += '%';
                auto uc = char_cast<uint8_t>(c);
                if (auto nibble = narrow_cast<char>(uc >> 4); nibble <= 9) {
                    r += '0' + nibble;
                } else {
                    r += 'A' + nibble - 10;
                }
                if (auto nibble = narrow_cast<char>(uc & 0xf); nibble <= 9) {
                    r += '0' + nibble;
                } else {
                    r += 'A' + nibble - 10;
                }
            }
            // clang-format on
        }

        return r;
    }

    [[nodiscard]] constexpr friend std::string to_string(URI const& rhs) noexcept
    {
        return rhs.normalized_string();
    }

private:
    using const_iterator = std::string::const_iterator;
    struct raw {};

    std::string _str;

    uint16_t _scheme_size;
    uint16_t _userinfo_size;
    uint16_t _host_size;
    uint16_t _port_size;
    uint16_t _path_size;
    uint16_t _query_size;
    uint16_t _fragment_size;
    uint16_t _num_segments;

    uint8_t _has_scheme : 1;
    uint8_t _has_host : 1;
    uint8_t _has_userinfo : 1;
    uint8_t _has_port : 1;
    uint8_t _has_query : 1;
    uint8_t _has_fragment : 1;
    uint8_t _path_is_absolute : 1;
    uint8_t _path_is_directory : 1;

    constexpr URI(raw, std::string_view str) : _str(str)
    {
        parse();
    }

    [[nodiscard]] constexpr uint16_t userinfo_offset() const noexcept
    {
        // [ scheme ":" ] [ "//" ]
        return _has_scheme + _scheme_size + (_has_host << 1);
    }

    [[nodiscard]] constexpr uint16_t host_offset() const noexcept
    {
        // userinfo_offset() [ userinfo "@" ]
        return userinfo_offset() + _userinfo_size + _has_userinfo;
    }

    [[nodiscard]] constexpr uint16_t port_offset() const noexcept
    {
        // host_offset() [ host ] [ ":" ]
        return host_offset() + _host_size + _has_port;
    }

    [[nodiscard]] constexpr uint16_t path_offset() const noexcept
    {
        // port_offset() [ port ]
        return port_offset() + _port_size;
    }

    [[nodiscard]] constexpr uint16_t query_offset() const noexcept
    {
        // path_offset() [ path ] [ "?" ]
        return path_offset() + _path_size + _has_query;
    }

    [[nodiscard]] constexpr uint16_t fragment_offset() const noexcept
    {
        // query_offset() [ query ] [ "#" ]
        return query_offset() + _query_size + _has_fragment;
    }

    [[nodiscard]] constexpr std::string_view subview(uint16_t pos, uint16_t count) const noexcept
    {
        return std::string_view{_str}.substr(pos, count);
    }

    [[nodiscard]] constexpr std::string_view raw_scheme() const noexcept
    {
        return subview(0, _scheme_size);
    }

    [[nodiscard]] constexpr std::string_view raw_userinfo() const noexcept
    {
        return subview(userinfo_offset(), _userinfo_size);
    }

    [[nodiscard]] constexpr std::string_view raw_host() const noexcept
    {
        return subview(host_offset(), _host_size);
    }

    [[nodiscard]] constexpr std::string_view raw_port() const noexcept
    {
        return subview(port_offset(), _port_size);
    }

    [[nodiscard]] constexpr std::string_view raw_path() const noexcept
    {
        return subview(path_offset(), _path_size);
    }

    [[nodiscard]] constexpr std::string_view raw_query() const noexcept
    {
        return subview(query_offset(), _query_size);
    }

    [[nodiscard]] constexpr std::string_view raw_fragment() const noexcept
    {
        return subview(fragment_offset(), _fragment_size);
    }

    [[nodiscard]] constexpr std::optional<std::string> get_scheme() const
    {
        if (_has_scheme) {
            return to_lower(raw_scheme());
        } else {
            return {};
        }
    }

    [[nodiscard]] constexpr std::optional<std::string> get_userinfo() const
    {
        hi_axiom(_has_host);
        if (_has_userinfo) {
            return decode(raw_userinfo());
        } else {
            return {};
        }
    }

    [[nodiscard]] constexpr std::optional<std::string> get_host() const
    {
        if (_has_host) {
            return decode(raw_host());
        } else {
            return {};
        }
    }

    [[nodiscard]] constexpr std::optional<std::string> get_port() const
    {
        if (_has_port) {
            return std::string{raw_port()};
        } else {
            return {};
        }
    }

    [[nodiscard]] constexpr std::optional<std::string> get_query() const
    {
        if (_has_query) {
            return decode(raw_query());
        } else {
            return {};
        }
    }

    [[nodiscard]] constexpr std::optional<std::string> get_fragment() const
    {
        if (_has_fragment) {
            return decode(raw_fragment());
        } else {
            return {};
        }
    }

    [[nodiscard]] constexpr bool check_scheme_start(char c) const noexcept
    {
        return (c >= 'a' and c <= 'z');
    }

    [[nodiscard]] constexpr bool check_scheme(std::string_view str) const noexcept
    {
        for (hilet c : str) {
            if (not((c >= 'a' and c <= 'z') or (c >= '0' and c <= '9') or c == '+' or c == '-' or c == '.')) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr bool check_port(std::string_view str) const noexcept
    {
        for (hilet c : str) {
            if (not(c >= '0' and c <= '9')) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr std::string normalized_string() const
    {
#define HI_SUB_DELIM '!', '$', '&', '\'', '(', ')', '*', '+', ',', ';', '='
#define HI_PCHAR HI_SUB_DELIM, ':', '@'
        auto r = std::string{};
        r.reserve(_str.size());

        if (auto scheme = get_scheme()) {
            // get_scheme() already returns a scheme in lower-case.
            if (not(check_scheme_start(scheme->front()) and check_scheme(*scheme))) {
                throw uri_error("Unexpected characters in scheme-component.");
            }

            r += encode<>(*scheme);
            r += ':';
        }

        hilet segments_ = segments();

        if (auto host = get_host()) {
            if (not(segments_.empty() or _path_is_absolute)) {
                throw uri_error("A path-component in a URI with an authority-component must be empty or absolute.");
            }

            r += '/';
            r += '/';
            if (auto userinfo = get_userinfo()) {
                r += encode<HI_SUB_DELIM, ':'>(*userinfo);
                r += '@';
            }

            if (host->empty() or host->front() != '[') {
                r += encode<HI_SUB_DELIM>(*host);
            } else {
                r += encode<HI_SUB_DELIM, '[', ']', ':'>(*host);
            }

            if (auto port = get_userinfo()) {
                if (not check_port(*port)) {
                    throw uri_error("Unexpected characters in port-component.");
                }
                r += ':';
                r += *port;
            }

        } else if (not segments_.empty() and segments_.front().empty()) {
            throw uri_error("A path-component in a URI without an authority-component may not start with a double slash '//'.");
        }

        auto first_segment = true;
        for (hilet& segment : segments()) {
            if (std::exchange(first_segment, false)) {
                if (_path_is_absolute) {
                    r += '/';
                    r += encode<HI_PCHAR>(segment);
                } else if (_has_scheme) {
                    r += encode<HI_PCHAR>(segment);
                } else {
                    r += encode<HI_SUB_DELIM, '@'>(segment);
                }

            } else {
                r += '/';
                r += encode<HI_PCHAR>(segment);
            }
        }

        if (auto query = get_query()) {
            r += '?';
            r += encode<HI_PCHAR, '/', '?'>(*query);
        }

        if (auto fragment = get_fragment()) {
            r += '#';
            r += encode<HI_PCHAR, '/', '?'>(*fragment);
        }

        return r;

#undef HI_PCHAR
#undef HI_SUB_DELIM
    }

    /** Parse a URI scheme part.
     *
     * @param str Substring of the URI starting at the potential scheme.
     * @return The number of characters in the scheme.
     * @retval -1 There is no scheme.
     */
    [[nodiscard]] constexpr const_iterator parse_scheme(const_iterator first, const_iterator last) noexcept
    {
        for (auto it = first; it != last; ++it) {
            if (hilet c = *it; c == ':') {
                _scheme_size = narrow_cast<uint16_t>(std::distance(first, it));
                _has_scheme = 1;
                return it + 1; // Skip over ':'.

            } else if (c == '/' or c == '?' or c == '#') {
                // Invalid character, this is not a scheme.
                return first;
            }
        }

        // Reached at end of URI, this is not a scheme.
        return first;
    }

    /** Parse a URI userinfo part.
     *
     * @param str Substring of the URI starting at the potential userinfo.
     * @return The number of characters in the userinfo.
     * @retval 0 The userinfo is an empty string.
     * @retval -1 There is no userinfo
     */
    [[nodiscard]] constexpr const_iterator parse_userinfo(const_iterator first, const_iterator last) noexcept
    {
        hi_axiom(_has_host);

        for (auto it = first; it != last; ++it) {
            if (hilet c = *it; c == '@') {
                _userinfo_size = narrow_cast<uint16_t>(std::distance(first, it));
                _has_userinfo = 1;
                return it + 1; // Skip over '@'.

            } else if (c == '/' or c == '?' or c == '#') {
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
    [[nodiscard]] constexpr const_iterator parse_host(const_iterator first, const_iterator last) noexcept
    {
        hi_axiom(_has_host);

        if (first == last) {
            return first;
        }

        auto it = first;
        if (*it == '[') {
            while (it != last) {
                if (*it == ']') {
                    _host_size = narrow_cast<uint16_t>(std::distance(first, it + 1));
                    return it + 1; // Skip over ']'.
                }
            }
            // End of URI mean this is not a host, interpret as path instead.
            return first;

        } else {
            for (; it != last; ++it) {
                if (hilet c = *it; c == ':' or c == '/' or c == '?' or c == '#') {
                    // End of host.
                    _host_size = narrow_cast<uint16_t>(std::distance(first, it));
                    return it;
                }
            }

            // End of URI means that the host is the last part of the URI
            _host_size = narrow_cast<uint16_t>(std::distance(first, last));
            return last;
        }
    }

    [[nodiscard]] constexpr const_iterator parse_port(const_iterator first, const_iterator last) noexcept
    {
        for (auto it = first; it != last; it++) {
            if (hilet c = *it; c == '/' or c == '?' or c == '#') {
                _port_size = narrow_cast<uint16_t>(std::distance(first, it));
                _has_port = 1;
                return it;
            }
        }

        _port_size = narrow_cast<uint16_t>(std::distance(first, last));
        _has_port = 1;
        return last;
    }

    [[nodiscard]] constexpr const_iterator parse_path(const_iterator first, const_iterator last) noexcept
    {
        if (first == last) {
            _path_is_directory = 1;
            return last;
        }

        // For relative paths a segment precedes the first '/'.
        _num_segments = 1;
        for (auto it = first; it != last; it++) {
            if (hilet c = *it; c == '/') {
                if (it == first) {
                    _path_is_absolute = 1;
                    // For absolute paths there is no segment preceding the leading '/'.
                    _num_segments = 0;
                }

                // '/' is always followed by a segment.
                ++_num_segments;

            } else if (c == '?' or c == '#') {
                _path_size = narrow_cast<uint16_t>(std::distance(first, it));
                _path_is_directory = narrow_cast<uint8_t>(*(it - 1) == '/');
                return it;
            }
        }

        _path_size = narrow_cast<uint16_t>(std::distance(first, last));
        _path_is_directory = narrow_cast<uint8_t>(*(last - 1) == '/');
        return last;
    }

    [[nodiscard]] constexpr const_iterator parse_query(const_iterator first, const_iterator last) noexcept
    {
        for (auto it = first; it != last; it++) {
            if (hilet c = *it; c == '#') {
                _query_size = narrow_cast<uint16_t>(std::distance(first, it));
                _has_query = 1;
                return it;
            }
        }

        _query_size = narrow_cast<uint16_t>(std::distance(first, last));
        _has_query = 1;
        return last;
    }

    [[nodiscard]] constexpr const_iterator parse_fragment(const_iterator first, const_iterator last) noexcept
    {
        _fragment_size = narrow_cast<uint16_t>(std::distance(first, last));
        _has_fragment = 1;
        return last;
    }

    constexpr void parse()
    {
        _has_scheme = 0;
        _has_userinfo = 0;
        _has_host = 0;
        _has_port = 0;
        _has_query = 0;
        _has_fragment = 0;

        _scheme_size = 0;
        _userinfo_size = 0;
        _host_size = 0;
        _port_size = 0;
        _path_size = 0;
        _query_size = 0;
        _fragment_size = 0;

        _path_is_absolute = 0;
        _path_is_directory = 0;
        _num_segments = 0;

        if (_str.size() >= std::numeric_limits<uint16_t>::max()) {
            throw uri_error("URI is larger than 65535 bytes.");
        }

        auto first = _str.cbegin();
        auto last = _str.cend();
        auto it = parse_scheme(first, last);

        if (std::distance(it, last) >= 2 and it[0] == '/' and it[1] == '/') {
            it += 2;

            _has_host = 1;
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
}} // namespace hi::v1
