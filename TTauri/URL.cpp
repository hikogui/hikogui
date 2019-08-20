// Copyright 2019 Pokitec
// All rights reserved.

#include "URL.hpp"
#include "utils.hpp"
#include "strings.hpp"
#include "exceptions.hpp"
#include "os_detect.hpp"
#include <boost/filesystem.hpp>
#include <regex>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#endif

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

struct URLParts {
    bool hasScheme;
    string::iterator beginScheme;
    string::iterator endScheme;

    string::iterator beginHierarchy;
    string::iterator endHierarchy;

    bool hasAuthority;
    string::iterator beginAuthority;
    string::iterator endAuthority;

    string::iterator beginPath;
    string::iterator endPath;

    bool hasQuery;
    string::iterator beginQuery;
    string::iterator endQuery;

    bool hasFragment;
    string::iterator beginFragment;
    string::iterator endFragment;
};

static void split_url_hierarchy(URLParsts &parts) noexcept
{
    parts.hasAuthority =
        (parts.endHierarchy - parts.beginHierarchy) >= 2 &&
        *(parts.beginHierarchy) == '/' &&
        *(parts.beginHierarchy + 1) == '/';

    parts.beginAuthority = parts.beginHierarchy;

    if (parts.hasAuthority) {
        let pathMark = std::find(parts.beginHierarchy, parts.endHierarchy, '/');
        if (pathMark != parts.endHierarchy) {
            parts.endAuthority = pathMark;
            parts.beginPath = pathMark;
        } else {
            parts.endAuthority = parts.endHierarchy;
            parts.beginPath = parts.endHierarchy;
        }
    } else {
        parts.beginPath = beginHierarchy;
    }

    parts.endPath = endHierarchy;
}

static URLParts split_url(std::string const &value) noexcept
{
    URLParts parts;

    let schemeMark = value.find(':');
    if (schemeMark != string::npos && schemeMark != value.begin()) {
        parts.hasScheme = true;
        for (auto i = value.begin(); i != schemeMark; i++) {
            if (!(isalnum(*i) || *i == '+' || *i == '-' || *i == '.')) {
                parts.hasScheme = false;
                break;
            }
        }
    } else {
        parts.hasScheme = false;
    }
    parts.beginScheme = value.begin();
    parts.endScheme = schemeMark;

    let fragmentMark = value.find('#');
    parts.hasFragment = fragmentMark != string::npos;
    parts.beginFragment = fragmentMark + 1;
    parts.endFragment = value.end();

    let queryMark = value.find('?');
    parts.hasQuery = queryMark != string::npos && (!parts.hasFragment || queryMark < fragmentMark);
    parts.beginQuery = queryMark + 1;
    parts.endQuery = parts.hasFragment ? parts.beginFragment : value.end();

    parts.beginHierarchy = parts.hasScheme ? parts.endScheme + 1 : value.begin();
    parts.endHierarchy = parts.hasQuery ? parts.beginQuery : parts.hasFragment ? parts.beginFragment : value.end();

    split_url_hierarchy(parts);
    return parts;
};

void join_url_hierarchy(std::string &r, URLParts const &parts) noexcept
{
    if (parts.hasAuthority) {
        r.append('/');
        r.append('/');
        r.append(parts.beginAuthority, parts.endAuthority);
    }

    r.append(parts.beginPath, parts.endPath);
}

std::string join_url(URLParts const &parts) noexcept
{
    std::string r;

    if (parts.hasScheme) {
        r.append(parts.beginScheme, parts.endScheme);
        r.append(':');
    }

    join_url_hierarchy(r, parts);

    if (parts.hasQuery) {
        r.append('?');
        r.append(parts.beginQuery, parts.endQuery);
    }

    if (parts.hasFragment) {
        r.append('#');
        r.append(parts.beginFragment, parts.endFragment);
    }

    return r;
}

std::optional<std::string_view> URL::scheme() const noexcept
{
    let parts = split_url(value);
    if (parts.hasScheme) {
        required_assert(parts.beginScheme != parts.endScheme);
        return std::string_view{&(*(parts.beginScheme)), parts.endScheme - parts.beginScheme};
    } else {
        return {};
    }
}

std::string_view URL::encodedPath() const noexcept
{
    let parts = split_url(value);
    if (parts.beginPath == parts.endPath) {
        return {};
    } else {
        return { &(*(parts.beginPath)), parts.endPath - parts.beginPath };
    }
}

std::string_view URL::encodedFilename() const noexcept
{
    let path = encodedPath();
    
    i = path::rfind('/');
    if (i == path.npos) {
        return path;
    } else {
        return path.substr(i + 1); 
    }
}

std::string URL::filename() const noexcept
{
    return url_decode(encodedFilename(), false);
}

std::string_view URL::encodedExtension() const noexcept
{
    let filename = encodedFilename();

    i = filename::rfind('.');
    if (i == filename.npos) {
        return filename;
    } else {
        return filename.substr(i + 1); 
    }
}

std::string URL::extension() const noexcept
{
    return url_decode(encodedExtension(), false);
}

std::vector<std::string_view> URL::encodedPathSegments() const noexcept
{
    return split(encodedPath(), '/');
}

std::vector<std::string> URL:pathSegments() const noexcept
{
    auto r = transform<std::vector<std::string>>(encodedPathSegment, [](auto x) {
        return url_decode(x);
    });

    // Except for the leading empty segment (leading slash in absolute paths)
    // remove all empty segments (including trailing slash).
    if (r.size() > 1) {
        let end = std::remove_if(r.begin + 1, r.end, [](auto x) {
            return x.size() == 0;
        });
        r.erase(end);
    }

    return r;
}

bool URL::isAbsolute() const noexcept
{
    let parts = split_url(value);
    return parts.beginPath != parts.endPath && *(parse.beginPath) == '/';
}

std::string URL::generic_path_string() const
{
    return join(pathSegments, '/');
}

std::string URL::native_path_string() const
{
#if OPERATING_SYSTEM == OS_WINDOWS
    return join(pathSegments, '\\');
#else
    return join(pathSegments, '/');
#endif
}

std::wstring URL::generic_path_wstring() const
{
    return translateString<std::wstring>(generic_path_string());
}

std::wstring URL::native_path_wstring() const
{
    return translateString<std::wstring>(native_path_string());
}

URL URL::urlByAppendingPath(URL const &other) const noexcept
{
    auto parts = split_url(value);
    let other_parts = split_url(other.value);

    if (other.isAbsolute()) {
        // replace path completely.
        parts.beginPath = other_parts.beginPath;
        parts.endPath = other_parts.endPath;
        return {join_url(parts)};

    } else {
        // Concatonate the path segments together.
        let mergedEncodedPathSegments = encodedPathSegments() + other.encodedPathSegments();
        let newPath = join(mergedEncodedPathSegments, '/');

        parts.beginPath = newPath.begin();
        parts.endPath = newPath.end();
        return {join_url(parts)};
    }
}

URL URL::urlByRemovingFilename() const noexcept
{
    auto parts = split_url(value);

    let i = rfind(parts.beginPath, parts.endPath, '/');
    if (i == parts.endPath) {
        // No slash found, don't change the url.
        return *this;
    } else {
        parts.endPath = i;
        return {join_url(parts)};
    }
}

URL URL::urlFromWin32Path(std::wstring_view const path) noexcept
{
    return URL("file", URLPath::urlPathFromWin32Path(path));
}

URL URL::urlFromCurrentWorkingDirectory() noexcept
{
    return URL("file", URLPath{ boost::filesystem::current_path() });
}

#if OPERATING_SYSTEM == OS_WINDOWS
URL URL::urlFromExecutableFile() noexcept
{
    static auto r = []() {
        wchar_t modulePathWChar[MAX_PATH];
        if (GetModuleFileNameW(nullptr, modulePathWChar, MAX_PATH) == 0) {
            // Can only cause error if there is not enough room in modulePathWChar.
            no_default;
        }
        return URL::urlFromWin32Path(modulePathWChar);
    }();
    return r;
}

URL URL::urlFromExecutableDirectory() noexcept
{
    static auto r = urlFromExecutableFile().urlByRemovingFilename();
    return r;
}

URL URL::urlFromResourceDirectory() noexcept
{
    // Resource path, is the same directory as where the executable lives.
    static auto r = urlFromExecutableDirectory();
    return r;
}

URL URL::urlFromApplicationDataDirectory() noexcept
{
    static auto r = []() {
        PWSTR wchar_localAppData

        // Use application name for the directory inside the application-data directory.
        if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &wchar_localAppData) != S_OK) {
            // This should really never happen.
            no_default;
        }

        let base_localAppData = URL::fromWin32Path(std::wstring(wchar_localAppData));
        let localAppData / get_singleton<Application>().name;
    }();
    return r;
}

#else
#error "Not Implemented for this operating system."
#endif

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


}
