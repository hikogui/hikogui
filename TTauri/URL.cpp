// Copyright 2019 Pokitec
// All rights reserved.

#include "URL.hpp"
#include "utils.hpp"
#include "strings.hpp"
#include "exceptions.hpp"
#include <regex>
#include <filesystem>

namespace TTauri {

static std::string percent_encode(char c)
{
    auto _c = bit_cast<uint8_t>(c);

    auto s = std::string{"%"};
    s += nibble_to_char((_c >> 4) & 0xf);
    s += nibble_to_char(_c & 0xf);
    return s;
}

std::string url_encode(std::string const &input, std::string_view const unreservedCharacters)
{
    std::string s;
    s.reserve(input.size());

    for (let c: input) {
        if (unreservedCharacters.find(c) != std::string_view::npos) {
            // Unreserved character.
            s += c;
        } else {
            s += percent_encode(c);
        }
    }

    return s;
}

std::string url_decode(std::string_view const &input, bool plusToSpace)
{
    enum class state_t {Idle, FirstNibble, SecondNibble};

    state_t state = state_t::Idle;

    auto s = std::string{};

    uint8_t v;
    for (let c: input) {
        switch (state) {
        case state_t::Idle:
            switch (c) {
            case '+':
                s+= plusToSpace ? ' ' : '+';
                break;

            case '%':
                state = state_t::FirstNibble;
                break;

            default:
                s+= c;
            }
            break;

        case state_t::FirstNibble:
            v = char_to_nibble(c) << 4;
            state = state_t::SecondNibble;
            break;

        case state_t::SecondNibble:
            v |= char_to_nibble(c);
            s += bit_cast<char>(v);
            state = state_t::Idle;
            break;

        default:
            no_default;
        }
    }

    return s;
}

URLUserinfo::URLUserinfo(std::string const &userinfo)
{
    auto i = userinfo.find(':');
    if (i != std::string_view::npos) {
        username = url_decode(userinfo.substr(0, i));
        password = url_decode(userinfo.substr(i + 1));
    } else {
        username = url_decode(userinfo);
        password = {};
    }
}


std::string to_string(URLUserinfo const &userinfo) {
    auto s = url_encode(userinfo.username, TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS);

    if (userinfo.password) {
        s += ":" + url_encode(userinfo.password.value(), TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS);
    }
    return s;
}

bool operator==(URLUserinfo const &lhs, URLUserinfo const &rhs)
{
    return std::tie(lhs.username, lhs.password) == std::tie(rhs.username, rhs.password);
}

bool operator<(URLUserinfo const &lhs, URLUserinfo const &rhs)
{
    return std::tie(lhs.username, lhs.password) < std::tie(rhs.username, rhs.password);
}

const auto URLAuthority_re = std::regex{"^(([^@]*)@)?(([[][^]]*[]])|([^:]*))(:([0-9]+))?", std::regex::extended};
//                                        12         34             5       6 7

URLAuthority::URLAuthority(std::string const &authority)
{
    std::smatch m;

    if (!std::regex_match(authority.begin(), authority.end(), m, URLAuthority_re)) {
        BOOST_THROW_EXCEPTION(URLError("Could not parse URLAuthority")
            << errinfo_parse_string(authority)
        );
    }

    required_assert(m.ready());
    let userinfo_sm = m[2];
    let host_sm = m[3];
    let port_sm = m[7];

    if (userinfo_sm.length()) {
        userinfo = URLUserinfo{userinfo_sm.str()};
    }
    host = url_decode(host_sm.str());
    if (port_sm.length()) {
        port = port_sm.str();
    }
}

std::string to_string(URLAuthority const &authority) {
    auto s = std::string{};

    if (authority.userinfo) {
        s += to_string(authority.userinfo.value()) + "@";
    }

    if (authority.host.front() == '[' && authority.host.back() == ']') {
        s += url_encode(authority.host, TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS "[]:");
    } else {
        s += url_encode(authority.host, TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS);
    }

    if (authority.port) {
        s += ":" + url_encode(authority.host, TTAURI_URL_UNRESERVED TTAURI_URL_SUB_DELIMS);
    }

    return s;
}

bool operator==(URLAuthority const &lhs, URLAuthority const &rhs)
{
    return std::tie(lhs.userinfo, lhs.host, lhs.port) == std::tie(rhs.userinfo, rhs.host, rhs.port);
}

bool operator<(URLAuthority const &lhs, URLAuthority const &rhs)
{
    return std::tie(lhs.userinfo, lhs.host, lhs.port) < std::tie(rhs.userinfo, rhs.host, rhs.port);
}

URLPath::URLPath(std::string const &path)
{
    absolute = (path.front() == '/');

    let encodedSegments = split(path.substr(absolute ? 1 : 0), '/');
    segments = transform<std::vector<std::string>>(encodedSegments, [](auto x) {
        return url_decode(x);
    });
}

URLPath::URLPath(std::filesystem::path const &path)
{
    absolute = false;
    for (let &segment: path) {
        if (segment == "/") {
            absolute = true;
        } else {
            segments.push_back(segment.string());
        }
    }
}

URLPath URLPath::fromWin32Path(std::wstring_view const &path_wstring)
{
    let path = std::filesystem::path(path_wstring);
    return URLPath(path);
}

std::string URLPath::string_path() const
{
    auto s = std::string{};
    s.reserve(std::accumulate(segments.begin(), segments.end(), segments.size() + 1, [](auto a, auto x) {
        return a + x.size();
        }));

    // Don't add a first slash if there is a drive letter first.
    auto addFirstSlash = absolute;
    if (segments.size() > 0 && segments.front().back() == ':') {
        addFirstSlash = false;
    }

    if (addFirstSlash) {
        s += "/";
    }

    for (size_t i = 0; i < segments.size(); i++) {
        if (i > 0) {
            s += "/";
        }
        s += segments.at(i);
    }

    return s;
}


std::string const &URLPath::filename() const
{
    if (segments.size() == 0) {
        BOOST_THROW_EXCEPTION(URLError("URLPath does not contain a filename"));
    }

    return segments.back();
}

std::string URLPath::extension() const
{
    let _filename = filename();

    let i = _filename.find('.');
    if (i != std::string::npos) {
        return _filename.substr(i + 1);
    } else {
        return {};
    }
}

std::string to_string(URLPath const &path)
{
    auto s = std::string{};
    s.reserve(std::accumulate(path.segments.begin(), path.segments.end(), path.segments.size() + 1, [](auto a, auto x) {
        return a + x.size();
    }));

    if (path.absolute) {
        s += "/";
    }

    for (size_t i = 0; i < path.segments.size(); i++) {
        if (i > 0) {
            s += "/";
        }
        s += url_encode(path.segments.at(i), TTAURI_URL_PCHAR);
    }

    return s;
}

bool operator==(URLPath const &lhs, URLPath const &rhs)
{
    return std::tie(lhs.absolute, lhs.segments) == std::tie(rhs.absolute, rhs.segments);
}

bool operator<(URLPath const &lhs, URLPath const &rhs)
{
    return std::tie(lhs.absolute, lhs.segments) < std::tie(rhs.absolute, rhs.segments);
}

const auto URL_re = std::regex{R"raw(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)raw", std::regex::extended};
URL::URL(std::string const &url)
{
    std::smatch m;

    if (!std::regex_match(url.begin(), url.end(), m, URL_re)) {
        BOOST_THROW_EXCEPTION(URLError("Could not parse URL")
            << errinfo_parse_string(url)
        );
    }

    required_assert(m.ready());
    let scheme_sm = m[2];
    let authority_sm = m[4];
    let path_sm = m[5];
    let query_sm = m[7];
    let fragment_sm = m[9];

    scheme = url_decode(scheme_sm.str());
    if (authority_sm.length()) {
        authority = URLAuthority{authority_sm.str()};
    }
    path = URLPath{path_sm.str()};
    if (query_sm.length()) {
        query = url_decode(query_sm.str(), true);
    }
    if (fragment_sm.length()) {
        fragment = url_decode(fragment_sm.str(), true);
    }
}

URL::URL(std::string scheme, URLPath path) : scheme(std::move(scheme)), path(std::move(path)) {}

std::string URL::string_path() const
{
    if (scheme != "file") {
        BOOST_THROW_EXCEPTION(URLError("URL is not a file.")
            << errinfo_url(*this)
        );
    }

    return path.string_path();
}

std::wstring URL::wstring_path() const
{
    return translateString<std::wstring>(string_path());
}

std::string const &URL::filename() const
{
    return path.filename();
}

std::string URL::extension() const
{
    return path.extension();
}

URL URL::urlByAppendingPath(URL const &other) const
{
    URL r = *this;

    if (other.path.absolute) {
        // replace path completely.
        r.path.segments = other.path.segments;
    } else {
        // append path segments.
        r.path.segments.insert(r.path.segments.end(), other.path.segments.begin(), other.path.segments.end());
    }

    return r;
}

std::string to_string(URL const &url) {
    auto s = url_encode(url.scheme, TTAURI_URL_ALPHA TTAURI_URL_DIGIT "+-.");

    if (url.authority) {
        s += to_string(url.authority.value());
    }

    s += to_string(url.path);

    if (url.query) {
        s += "?" + url_encode(*url.query, TTAURI_URL_PCHAR "/?");
    }

    if (url.fragment) {
        s += "?" + url_encode(*url.fragment, TTAURI_URL_PCHAR "/?");
    }

    return s;
}

std::filesystem::path path(URL const &url)
{
    return { url.string_path() };
}

size_t file_size(URL const &url)
{
    return std::filesystem::file_size(path(url));    
}

bool operator==(URL const &lhs, URL const &rhs)
{
    return
        std::tie(lhs.scheme, lhs.authority, lhs.path, lhs.query, lhs.fragment) ==
        std::tie(rhs.scheme, rhs.authority, rhs.path, rhs.query, rhs.fragment);
}

bool operator<(URL const &lhs, URL const &rhs)
{
    return
        std::tie(lhs.scheme, lhs.authority, lhs.path, lhs.query, lhs.fragment) <
        std::tie(rhs.scheme, rhs.authority, rhs.path, rhs.query, rhs.fragment);
}

}