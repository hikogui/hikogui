// Copyright 2019 Pokitec
// All rights reserved.

#include "URL.hpp"
#include "utils.hpp"
#include "strings.hpp"
#include "exceptions.hpp"
#include <boost/filesystem.hpp>
#include <regex>

namespace TTauri {

static std::string percent_encode(char c) noexcept
{
    auto _c = bit_cast<uint8_t>(c);

    auto s = std::string{"%"};
    s += nibble_to_char((_c >> 4) & 0xf);
    s += nibble_to_char(_c & 0xf);
    return s;
}

std::string url_encode(std::string const &input, std::string_view const unreservedCharacters) noexcept
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

std::string url_decode(std::string_view const &input, bool plusToSpace) noexcept
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

    if (scheme_sm.length()) {
        scheme = url_decode(scheme_sm.str());
    } else {
        scheme = "file";
    }
    
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

URL::URL(char const *url) : URL(std::string(url)) {}

URL::URL(std::string const scheme, URLPath path) noexcept : scheme(std::move(scheme)), path(std::move(path)) {}

std::string URL::path_string() const
{
    if (scheme != "file") {
        BOOST_THROW_EXCEPTION(URLError("URL is not a file.")
            << errinfo_url(*this)
        );
    }

    return path.path_string();
}

std::wstring URL::path_wstring() const
{
    return translateString<std::wstring>(path_string());
}

std::string const &URL::filename() const
{
    return path.filename();
}

std::string URL::extension() const
{
    return path.extension();
}

bool URL::isAbsolute() const noexcept
{
    return path.absolute;
}

bool URL::isRelative() const noexcept
{
    return !path.absolute;
}

URL URL::urlByAppendingPath(URL const &other) const noexcept
{
    auto r = *this;

    if (other.path.absolute) {
        // replace path completely.
        r.path.segments = other.path.segments;
    } else {
        // append path segments.
        r.path.segments.insert(r.path.segments.end(), other.path.segments.begin(), other.path.segments.end());
    }

    return r;
}

URL URL::urlByRemovingFilename() const noexcept
{
    auto r = *this;

    if (r.path.segments.size() > 0) {
        r.path.segments.pop_back();
    }

    return r;
}

URL URL::urlFromWin32Path(std::wstring_view const path) noexcept
{
    return URL("file", URLPath::urlPathFromWin32Path(path));
}

URL URL::urlFromCurrentWorkingDirectory() noexcept
{
    return URL("file", URLPath{ boost::filesystem::current_path() });
}

std::string to_string(URL const &url) noexcept
{
    auto s = url_encode(url.scheme, TTAURI_URL_ALPHA TTAURI_URL_DIGIT "+-.") + ":";

    if (url.authority) {
        s += to_string(url.authority.value());
    }

    s += to_string(url.path);

    if (url.query) {
        s += "?" + url_encode(*url.query, TTAURI_URL_PCHAR "/?");
    }

    if (url.fragment) {
        s += "#" + url_encode(*url.fragment, TTAURI_URL_PCHAR "/?");
    }

    return s;
}

boost::filesystem::path path(URL const &url)
{
    return { url.path_string() };
}

size_t file_size(URL const &url)
{
    return boost::filesystem::file_size(path(url));    
}

bool operator==(URL const &lhs, URL const &rhs) noexcept
{
    return
        std::tie(lhs.scheme, lhs.authority, lhs.path, lhs.query, lhs.fragment) ==
        std::tie(rhs.scheme, rhs.authority, rhs.path, rhs.query, rhs.fragment);
}

bool operator<(URL const &lhs, URL const &rhs) noexcept
{
    return
        std::tie(lhs.scheme, lhs.authority, lhs.path, lhs.query, lhs.fragment) <
        std::tie(rhs.scheme, rhs.authority, rhs.path, rhs.query, rhs.fragment);
}

}