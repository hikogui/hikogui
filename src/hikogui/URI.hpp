// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "cast.hpp"
#include "exception.hpp"
#include "ranges.hpp"
#include "generator.hpp"
#include "fixed_string.hpp"
#include <string>
#include <optional>
#include <ranges>

namespace hi { inline namespace v1 {

#define HI_SUB_DELIM '!', '$', '&', '\'', '(', ')', '*', '+', ',', ';', '='
#define HI_PCHAR HI_SUB_DELIM, ':', '@'

/**
 *
 * @note Maximum size of a URI is 65535 octets
 */
class URI {
public:
    struct authority_type {
        std::optional<std::string> userinfo;
        std::string host;
        std::optional<std::string> port;
    };

    /** A path type.
     *
     *  Path string             | Segment list
     *  ----------------------- | -----------------
     *  ""                      | [""]
     *  "/"                     | ["", ""]
     *  "filename"              | ["filename"]
     *  "/filename"             | ["", "filename"]
     *  "dirname/"              | ["dirname", ""]
     *  "/dirname/"             | ["", "dirname", ""]
     *  "/dirname/filename"     | ["", "dirname", "filename"]
     *  "."                     | [".", ""]
     *  "/."                    | ["", ".", ""]
     *  "./"                    | [".", ""]
     *  "/./"                   | ["", ".", ""]
     *  "/./."                  | ["", ".", ".", ""]
     */
    class path_type : public std::vector<std::string> {
    public:
        constexpr path_type() noexcept
        {
            emplace_back();
        }

        constexpr path_type(path_type const&) noexcept = default;
        constexpr path_type(path_type&&) noexcept = default;
        constexpr path_type& operator=(path_type const&) noexcept = default;
        constexpr path_type& operator=(path_type&&) noexcept = default;

        constexpr path_type(std::string_view str) noexcept
        {
            reserve(std::ranges::count(str, '/') + 1);
            auto offset = 0_uz;

            // Check for leading slash.
            if (str.starts_with('/')) {
                ++offset;
                emplace_back();
            }

            // Find all directories.
            while (offset < str.size()) {
                hilet slash_index = str.find('/', offset);
                if (slash_index == std::string_view::npos) {
                    break;
                }
                push_back(URI::decode(str.substr(offset, slash_index - offset)));
                offset = slash_index + 1;
            }

            // Append filename.
            auto filename = URI::decode(str.substr(offset));
            hilet filename_is_directory = filename == "." or filename == "..";
            push_back(std::move(filename));

            // "." or ".." are directories, add an explicit trailing slash.
            if (filename_is_directory) {
                emplace_back();
            }
        }

        [[nodiscard]] constexpr bool empty() const noexcept
        {
            hi_axiom(size() != 0);
            return size() == 1 and front().empty();
        }

        [[nodiscard]] constexpr bool absolute() const noexcept
        {
            hi_axiom(size() != 0);
            return size() >= 2 and front().empty();
        }

        [[nodiscard]] constexpr bool double_absolute() const noexcept
        {
            hi_axiom(size() != 0);
            return size() >= 3 and (*this)[0].empty() and (*this)[1].empty();
        }

        [[nodiscard]] constexpr friend path_type merge(path_type base, path_type const& ref, bool base_has_authority) noexcept
        {
            hi_axiom(base.size() != 0 and ref.size() != 0);

            if (base_has_authority and base.empty()) {
                base.emplace_back();
                base.insert(base.cend(), ref.cbegin(), ref.cend());

            } else {
                if (not base.empty()) {
                    base.pop_back();
                }
                base.insert(base.cend(), ref.cbegin(), ref.cend());
            }
            return base;
        }

        /**
         *
         *  Path              | List                            | Result Path | Result List
         *  -------------     | ---------------------------     | ----------- | -----------
         *  ".."              | ["..", ""]                      | ""          | [""]
         *  "/.."             | ["", "..", ""]                  | ""          | [""]
         *  "foo/.."          | ["foo", "..", ""]               | ""          | [""]
         *  "/foo/.."         | ["", "foo", "..", ""]           | "/"         | ["", ""]
         *  "baz/foo/.."      | ["baz", "foo", "..", ""]        | "baz/"      | ["baz", ""]
         *  "/baz/foo/.."     | ["", "baz", "foo", "..", ""]    | "/baz/"     | ["", "baz", ""]
         *                    |                                 |             |
         *  "../"             | ["..", ""]                      | ""          | [""]
         *  "/../"            | ["", "..", ""]                  | ""          | [""]
         *  "foo/../"         | ["foo", "..", ""]               | ""          | [""]
         *  "/foo/../"        | ["", "foo", "..", ""]           | "/"         | ["", ""]
         *  "baz/foo/../"     | ["baz", "foo", "..", ""]        | "baz/"      | ["baz", ""]
         *  "/baz/foo/../"    | ["", "baz", "foo", "..", ""]    | "/baz/"     | ["", "baz", ""]
         *                    |                                 |             |
         *  "../bar"          | ["..", "bar"]                   | "bar"       | ["bar"]
         *  "/../bar"         | ["", "..", "bar"]               | "bar"       | ["bar"]
         *  "foo/../bar"      | ["foo", "..", "bar"]            | "bar"       | ["bar"]
         *  "/foo/../bar"     | ["", "foo", "..", "bar"]        | "/bar"      | ["", "bar"]
         *  "baz/foo/../bar"  | ["baz", "foo", "..", "bar"]     | "baz/bar"   | ["baz", "bar"]
         *  "/baz/foo/../bar" | ["", "baz", "foo", "..", "bar"] | "/baz/bar"  | ["", "baz", "bar"]
         */
        [[nodiscard]] constexpr friend path_type remove_dot_segments(path_type path) noexcept
        {
            for (auto it = path.cbegin(); it != path.cend();) {
                if (*it == ".") {
                    //  Remove any "." from the path.
                    it = path.erase(it);

                } else if (*it == "..") {
                    if (it == path.cbegin()) {
                        // Remove the ".." at the start of a relative path.
                        it = path.erase(it);

                    } else if (it - 1 == path.cbegin() and (it - 1)->empty()) {
                        // Remove just the ".." at the start of an absolute path.
                        it = path.erase(it);

                    } else {
                        // Remove ".." and the segment in front of it.
                        it = path.erase(it - 1, it + 1);
                    }
                } else {
                    // Ignore other segments.
                    ++it;
                }
            }

            return path;
        }

        [[nodiscard]] constexpr size_t encode_size() const noexcept
        {
            size_t r = 0_uz;
            for (hilet& segment : *this) {
                r += segment.size();
            }
            r += size() + 1;
            return r;
        }

        [[nodiscard]] constexpr std::string encode(bool has_scheme) const noexcept
        {
            hi_axiom(size() != 0);

            auto r = std::string{};
            r.reserve(encode_size());

            auto it = cbegin();
            if (absolute()) {
                r += '/';
                ++it;
            }

            auto first_segment = true;
            for (; it != cend(); ++it) {
                if (first_segment) {
                    first_segment = false;
                    if (absolute() or has_scheme) {
                        r += URI::encode<HI_PCHAR>(*it);
                    } else {
                        r += URI::encode<HI_SUB_DELIM, '@'>(*it);
                    }

                } else {
                    r += '/';
                    r += URI::encode<HI_PCHAR>(*it);
                }
            }

            return r;
        }
    };

    struct components_type {
        std::optional<std::string> scheme;
        std::optional<authority_type> authority;
        path_type path;
        std::optional<std::string> query;
        std::optional<std::string> fragment;

        [[nodiscard]] constexpr size_t encode_size(size_t size_hint) const noexcept
        {
            if (size_hint == 0) {
                if (scheme) {
                    size_hint += scheme->size();
                    size_hint += 1;
                }
                if (authority) {
                    size_hint += 2;
                    if (authority->userinfo) {
                        size_hint += authority->userinfo->size();
                        size_hint += 1;
                    }
                    size_hint += authority->host.size();
                    if (authority->port) {
                        size_hint += authority->port->size();
                        size_hint += 1;
                    }
                }
                size_hint += path.encode_size();
                size_hint += 1;

                if (query) {
                    size_hint += query->size();
                    size_hint += 1;
                }
                if (fragment) {
                    size_hint += fragment->size();
                    size_hint += 1;
                }
            }

            return size_hint;
        }

        /** Make a URI string from components.
         */
        [[nodiscard]] constexpr std::string encode(size_t size_hint = 0) const
        {
            auto r = std::string{};
            r.reserve(encode_size(size_hint));

            if (scheme) {
                // get_scheme() already returns a scheme in lower-case.
                if (not(URI::check_scheme_start(scheme->front()) and URI::check_scheme(*scheme))) {
                    throw uri_error("Unexpected characters in scheme-component.");
                }

                r += *scheme;
                r += ':';
            }

            if (authority) {
                if (not(path.empty() or path.absolute())) {
                    throw uri_error("A path-component in a URI with an authority-component must be empty or absolute.");
                }

                r += '/';
                r += '/';
                if (authority->userinfo) {
                    r += URI::encode<HI_SUB_DELIM, ':'>(*authority->userinfo);
                    r += '@';
                }

                if (authority->host.empty() or authority->host.front() != '[') {
                    r += URI::encode<HI_SUB_DELIM>(authority->host);
                } else {
                    r += URI::encode<HI_SUB_DELIM, '[', ']', ':'>(authority->host);
                }

                if (authority->port) {
                    if (not URI::check_port(*authority->port)) {
                        throw uri_error("Unexpected characters in port-component.");
                    }
                    r += ':';
                    r += *authority->port;
                }

            } else if (path.double_absolute()) {
                throw uri_error(
                    "A path-component in a URI without an authority-component may not start with a double slash '//'.");
            }

            r += path.encode(to_bool(scheme));

            if (query) {
                r += '?';
                r += URI::encode<HI_PCHAR, '/', '?'>(*query);
            }

            if (fragment) {
                r += '#';
                r += URI::encode<HI_PCHAR, '/', '?'>(*fragment);
            }

            return r;
        }
    };

    constexpr URI() noexcept = default;
    constexpr URI(URI const&) noexcept = default;
    constexpr URI(URI&&) noexcept = default;
    constexpr URI& operator=(URI const&) noexcept = default;
    constexpr URI& operator=(URI&&) noexcept = default;

    constexpr URI(std::in_place_t, std::string str) : _str(std::move(str))
    {
        parse();
    }

    constexpr URI(components_type const& components) noexcept : URI(std::in_place_t{}, components.encode()) {}

    /** Construct a URL from a string.
     *
     * @note This constructor will normalize the URI
     * @throws uri_error When the URI can not be normalized due to a parse error.
     */
    constexpr URI(std::string str) : URI(URI{std::in_place_t{}, std::move(str)}.get_components()) {}

    /** Construct a URL from a string.
     *
     * @note This constructor will normalize the URI
     * @throws uri_error When the URI can not be normalized due to a parse error.
     */
    constexpr URI(char const *str) : URI(std::string{str}) {}

    /** Construct a URL from a string.
     *
     * @note This constructor will normalize the URI
     * @throws uri_error When the URI can not be normalized due to a parse error.
     */
    constexpr URI(std::string_view str) : URI(std::string{str}) {}

    /** Get the scheme-component of the URI.
     *
     * @return The optional and lower-cased scheme-component.
     */
    [[nodiscard]] constexpr std::optional<std::string> scheme() const noexcept
    {
        return get_scheme();
    }

    /** Get the authority-component of the URI.
     *
     * @return The optional and decoded userinfo, host and port..
     */
    [[nodiscard]] constexpr std::optional<authority_type> authority() const noexcept
    {
        return get_authority();
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

    [[nodiscard]] constexpr path_type path() const noexcept
    {
        return get_path();
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

    /** Get the components of the URI.
     *
     * @return The full set of components.
     */
    [[nodiscard]] constexpr components_type components() const noexcept
    {
        return get_components();
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
        return rhs._str;
    }

    friend std::ostream& operator<<(std::ostream& lhs, URI const& rhs) noexcept
    {
        return lhs << rhs._str;
    }

    [[nodiscard]] constexpr friend bool operator==(URI const& lhs, URI const& rhs) noexcept
    {
        return lhs._str == rhs._str;
    }

    [[nodiscard]] constexpr friend auto operator<=>(URI const& lhs, URI const& rhs) noexcept
    {
        return lhs._str <=> rhs._str;
    }

    [[nodiscard]] constexpr friend URI operator/(URI const& base, URI const& ref) noexcept
    {
        auto T = components_type{};
        auto B = base.components();
        auto R = ref.components();

        // non-strict parses ignore scheme
        // if (R_scheme == B_scheme) {
        //    R_scheme = {};
        //}

        if (R.scheme) {
            T.scheme = R.scheme;
            T.authority = R.authority;
            T.path = remove_dot_segments(R.path);
            T.query = R.query;
        } else {
            if (R.authority) {
                T.authority = R.authority;
                T.path = remove_dot_segments(R.path);
                T.query = R.query;

            } else {
                if (R.path.empty()) {
                    T.path = B.path;
                    if (R.query) {
                        T.query = R.query;
                    } else {
                        T.query = B.query;
                    }
                } else {
                    if (R.path.absolute()) {
                        T.path = remove_dot_segments(R.path);
                    } else {
                        T.path = remove_dot_segments(merge(B.path, R.path, to_bool(B.authority)));
                    }
                    T.query = R.query;
                }
                T.authority = B.authority;
            }
            T.scheme = B.scheme;
        }

        T.fragment = R.fragment;

        return {T};
    }

private:
    using const_iterator = std::string::const_iterator;

    std::string _str = {};

    uint16_t _scheme_size = 0;
    uint16_t _userinfo_size = 0;
    uint16_t _host_size = 0;
    uint16_t _port_size = 0;
    uint16_t _path_size = 0;
    uint16_t _query_size = 0;
    uint16_t _fragment_size = 0;

    uint8_t _has_scheme : 1 = 0;
    uint8_t _has_host : 1 = 0;
    uint8_t _has_userinfo : 1 = 0;
    uint8_t _has_port : 1 = 0;
    uint8_t _has_query : 1 = 0;
    uint8_t _has_fragment : 1 = 0;

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

    [[nodiscard]] constexpr std::optional<authority_type> get_authority() const
    {
        if (_has_host) {
            return authority_type{get_userinfo(), decode(raw_host()), get_port()};
        } else {
            return {};
        }
    }

    [[nodiscard]] constexpr path_type get_path() const noexcept
    {
        return path_type{raw_path()};
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

    [[nodiscard]] constexpr components_type get_components() const
    {
        return {get_scheme(), get_authority(), get_path(), get_query(), get_fragment()};
    }

    [[nodiscard]] constexpr static bool check_scheme_start(char c) noexcept
    {
        return (c >= 'a' and c <= 'z');
    }

    [[nodiscard]] constexpr static bool check_scheme(std::string_view str) noexcept
    {
        for (hilet c : str) {
            if (not((c >= 'a' and c <= 'z') or (c >= '0' and c <= '9') or c == '+' or c == '-' or c == '.')) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr static bool check_port(std::string_view str) noexcept
    {
        for (hilet c : str) {
            if (not(c >= '0' and c <= '9')) {
                return false;
            }
        }
        return true;
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
            return last;
        }

        for (auto it = first; it != last; it++) {
            if (hilet c = *it; c == '?' or c == '#') {
                _path_size = narrow_cast<uint16_t>(std::distance(first, it));
                return it;
            }
        }

        _path_size = narrow_cast<uint16_t>(std::distance(first, last));
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

    friend struct std::hash<URI>;
};

#undef HI_PCHAR
#undef HI_SUB_DELIM

}} // namespace hi::v1

template<>
struct std::hash<hi::URI> {
    [[nodiscard]] size_t operator()(hi::URI const& rhs) const noexcept
    {
        return std::hash<std::string>{}(rhs._str);
    }
};

template<typename CharT>
struct std::formatter<hi::URI, CharT> : std::formatter<std::string, CharT> {
    auto format(hi::URI const& t, auto& fc)
    {
        return std::formatter<std::string, CharT>::format(to_string(t), fc);
    }
};
