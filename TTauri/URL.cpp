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

std::string url_encode(std::string_view const input, std::string_view const unreservedCharacters) noexcept
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
    bool hasScheme = false;
    std::string::const_iterator beginScheme;
    std::string::const_iterator endScheme;

    std::string::const_iterator beginHierarchy;
    std::string::const_iterator endHierarchy;

    bool hasAuthority = false;
    std::string::const_iterator beginAuthority;
    std::string::const_iterator endAuthority;

    std::string::const_iterator beginPath;
    std::string::const_iterator endPath;

    bool hasQuery = false;
    std::string::const_iterator beginQuery;
    std::string::const_iterator endQuery;

    bool hasFragment = false;
    std::string::const_iterator beginFragment;
    std::string::const_iterator endFragment;
};

static void split_url_hierarchy(URLParts &parts) noexcept
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
        parts.beginPath = parts.beginHierarchy;
    }

    parts.endPath = parts.endHierarchy;
}

static URLParts split_url(std::string const &value) noexcept
{
    URLParts parts;

    let schemeMark = std::find(value.begin(), value.end(), ':');
    if (schemeMark != value.end() && schemeMark != value.begin()) {
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

    let fragmentMark = std::find(value.begin(), value.end(), '#');
    parts.hasFragment = fragmentMark != value.end();
    parts.beginFragment = fragmentMark + 1;
    parts.endFragment = value.end();

    let queryMark = std::find(value.begin(), value.end(), '?');
    parts.hasQuery = queryMark != value.end() && (!parts.hasFragment || queryMark < fragmentMark);
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
        r.append(2, '/');
        r.append(parts.beginAuthority, parts.endAuthority);
    }

    r.append(parts.beginPath, parts.endPath);
}

std::string join_url(URLParts const &parts) noexcept
{
    std::string r;

    if (parts.hasScheme) {
        r.append(parts.beginScheme, parts.endScheme);
        r.append(1, ':');
    }

    join_url_hierarchy(r, parts);

    if (parts.hasQuery) {
        r.append(1, '?');
        r.append(parts.beginQuery, parts.endQuery);
    }

    if (parts.hasFragment) {
        r.append(1, '#');
        r.append(parts.beginFragment, parts.endFragment);
    }

    return r;
}

std::optional<std::string_view> URL::scheme() const noexcept
{
    let parts = split_url(value);
    if (parts.hasScheme) {
        return make_string_view(parts.beginScheme, parts.endScheme);
    } else {
        return {};
    }
}

std::optional<std::string_view> URL::encodedQuery() const noexcept
{
    let parts = split_url(value);
    if (parts.hasQuery) {
        return make_string_view(parts.beginQuery, parts.endQuery);
    } else {
        return {};
    }
}

std::optional<std::string> URL::query() const noexcept
{
    if (let view = encodedQuery()) {
        return url_decode(*view);
    } else {
        return {};
    }
}

std::optional<std::string_view> URL::encodedFragment() const noexcept
{
    let parts = split_url(value);
    if (parts.hasFragment) {
        return make_string_view(parts.beginFragment, parts.endFragment);
    } else {
        return {};
    }
}

std::optional<std::string> URL::fragment() const noexcept
{
    if (let view = encodedFragment()) {
        return url_decode(*view);
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
    
    let i = path::rfind('/');
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

    let i = filename::rfind('.');
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

std::vector<std::string> URL::pathSegments() const noexcept
{
    auto r = transform<std::vector<std::string>>(encodedPathSegments(), [](auto x) {
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
    return parts.beginPath != parts.endPath && *(parts.beginPath) == '/';
}

std::string URL::directory() const noexcept
{
    auto segments = pathSegments();
    segments.pop_back();
    return join(segments, '/');
}

std::string URL::path() const noexcept
{
    return join(pathSegments(), '/');
}

std::string URL::nativePath() const noexcept
{
#if OPERATING_SYSTEM == OS_WINDOWS
    return join(pathSegments, '\\');
#else
    return join(pathSegments, '/');
#endif
}

std::wstring URL::nativeWPath() const noexcept
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

struct PathParts {
    std::string_view server;
    std::string_view drive;
    bool absolute;
    std::vector<std::string_view> segments;
};

PathParts parse_path(std::vector<std::string_view> segments) noexcept
{
    PathParts parts;

    // Extract optional server from file path.
    if (segments.size() >= 3 && segments.at(0).size() == 0 && segments.at(1).size() == 0) {
        // Start with two slashes: UNC filename starting with a server.
        parts.server = segments.at(3);
        
        // Remove the server-name and leading double slash. But keep a leading slash in,
        // because what follows is an absolute path.
        segments.erase(segments.begin() + 1, segments.begin() + 3);
    }

    // Extract optional drive from file path.
    if (segments.size() >= 2 && segments.at(0).size() == 0 && segments.at(1).find(':') != std::string_view::npos) {
        // Due to how file URLs with authority requires absolute paths, it may be that the
        // drive letter follows a leading slash, but this does not mean it is an absolute path.
        let i = segments.at(1).find(':');
        parts.drive = segments.at(1).substr(0, i);
        segments.at(1) = segments.at(1).substr(i + 1);

    } else if (segments.size() >= 1 && segments.at(0).find(':') != std::string_view::npos) {
        // This is more sane, a drive letter as the first segment of a path.
        let i = segments.at(0).find(':');
        parts.drive = segments.at(0).substr(0, i);
        segments.at(0) = segments.at(0).substr(i + 1);
    }

    // Check for a leading slash '/' meaning an absolute path.
    parts.absolute = segments.size() >= 1 && segments.at(0).size() == 0;

    // Normalize the rest of the path.
    for (auto i = segments.begin(); i != segments.end(); i++) {
        if (i.size() == 0 || *i == '.' || (absolute && i == segments.begin() && *i == "..")) {
            // Strip out:
            //  * remove the leading slash "/foo/bar" -> "foo/bar"
            //  * double slashes "foo//bar" -> "foo/bar"
            //  * dot names "foo/./bar" -> "foo/bar"
            //  * and trailing slash "foo/" -> "foo"
            //  * and double dot at the start of an absolute path. "/../foo" -> "/foo"
            i = segments.erase(i, i + 1);
            
        } else if (*i != ".." && (i+1) != segments.end() && *(i+1) == "..") {
            // Remove both when a name is followed by a double dot:
            //  * "foo/bar/../baz" -> "foo/baz"
            i = segments.erase(i, i + 2);

            // Backtrack, because the previous could now be a name and the new next a double dot.
            //  * "hoi/foo/bar/../../baz" -> "hoi/foo/../baz" -> "hoi/baz"
            i = (i == segments.begin()) ? i : i - 1;
        }
    }

    parts.segments = std::move(segments);
    return parts;
}

PathParts parse_path(std::vector<std::string_view> segments) noexcept
{
    return parse_path(split(path, '/', '\\'));
}

std::string generate_path(PathParts const &parts) noexcept
{
    let size_guess = std::accumulate(
        parts.segments.begin(), parts.segments.end(),
        parts.server.size() + parts.drive.size() + parts.segments.size() + 4,
        [](size_t a, auto b) {
            return a + b.size();
        }
    );

    std::string r;
    r.capacity(size_guess);

    if (parts.server.size() > 0) {
        r.append(2, '/');
        r.append(parts.server);
    }

    if (parts.drive.size() > 0) {
        if (parts.server.size()) {
            r.append(1, '/');
        }
        r.append(parts.drive);
        r.append(1, ':');
    }

    if (parts.absolute) {
        r.append(1, '/');
    }

    r.append(join(parts.segments, '/'));
    return r;
}


URL URL::urlFromPath(std::string_view const path) noexcept
{
    URLParts parts;

    let scheme = std::string("file");
    parts.hasScheme = true;
    parts.beginScheme = scheme.begin();
    parts.endScheme = scheme.end();

    auto segments = split(path, '/', '\\');

    std::string authority;
    if (segments.size() >= 3 && segments.at(0).size() == 0 && segments.at(1).size() == 0) {
        // Start with two slashes: UNC filename starting with a server.
        authority = url_encode(segments.at(3), TTAURI_URL_HOST);
        parts.hasAuthority = true;
        parts.beginAuthority = authority.begin();
        parts.endAuthority = authority.end();
        // Remove the server-name and leading double slash. But keep a leading slash in,
        // because what follows is an absolute path.
        segments.erase(segments.begin() + 1, segments.begin() + 3);
    }
}

URL URL::urlFromWPath(std::wstring_view const path) noexcept
{
    return urlFromPath(translateString<std::string>(path));
}

URL URL::urlFromCurrentWorkingDirectory() noexcept
{
    return URL{ std::string("file:") + boost::filesystem::current_path().generic_string() };
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


size_t file_size(URL const &url)
{
    return boost::filesystem::file_size(boost::filesystem::path(url.path()));    
}


}
