// Copyright 2019 Pokitec
// All rights reserved.

#include "required.hpp"
#include "url_parser.hpp"
#include "strings.hpp"
#include "memory.hpp"
#include <numeric>

namespace tt {

static bool is_urlchar_scheme(char c, size_t i)
{
    return is_urlchar_alpha(c) || (i > 0 && (is_urlchar_digit(c) || c == '+' || c == '-' || c == '.'));
}

std::string url_encode_part(std::string_view const input, std::function<bool(char)> unreserved_char_check) noexcept
{
    std::string s;
    s.reserve(input.size() + input.size() / 2);

    for (ttlet c: input) {
        if (unreserved_char_check(c)) {
            // Unreserved character.
            s += c;
        } else {
            s += '%';
            s += nibble_to_char((c >> 4) & 0xf);
            s += nibble_to_char(c & 0xf);
        }
    }
    return s;
}

std::string url_decode(std::string_view const input, bool const plus_to_space) noexcept
{
    enum class state_t {Idle, FirstNibble, SecondNibble};

    state_t state = state_t::Idle;

    auto s = std::string{};

    uint8_t value = 0;
    int8_t nibble_result;
    for (ttlet c: input) {
        switch (state) {
        case state_t::Idle:
            switch (c) {
            case '+':
                s += plus_to_space ? ' ' : '+';
                break;

            case '%':
                state = state_t::FirstNibble;
                break;

            default:
                s += c;
            }
            break;

        case state_t::FirstNibble:
            nibble_result = char_to_nibble(c);
            if (nibble_result == -1) {
                // Not a nibble, pretent that there was no encoding.
                s += '%';
                s += c;
                state = state_t::Idle;
            } else {
                value = static_cast<uint8_t>(nibble_result) << 4;
                state = state_t::SecondNibble;
            }
            break;

        case state_t::SecondNibble:
            nibble_result = char_to_nibble(c);
            if (nibble_result == -1) {
                // Not a nibble, pretent that there was no encoding.
                s += '%';
                s += nibble_to_char(value >> 4);
                s += c;
                state = state_t::Idle;
            } else {
                value |= static_cast<uint8_t>(nibble_result);
                s += static_cast<char>(value);
                state = state_t::Idle;
            }
            break;

        default:
            tt_no_default();
        }
    }

    return s;
}

static void parse_authority_split(url_parts &parts, std::string_view authority) noexcept
{
    parts.authority = authority;
}

/*! Parse and normalize a file path.
* The path is already split into segments.
* This function will work with both url-encoded or no encoding paths.
*
* The input segments may include empty segments such as from the leading
* slash of an absolute path.
*
* \param segments a list of path segments.
* \return the path split into the parts and normalized.
*/
static void parse_path_split(url_parts &parts, std::vector<std::string_view> segments) noexcept
{
    // Extract optional server from file path.
    if (segments.size() >= 3 && segments.at(0).size() == 0 && segments.at(1).size() == 0) {
        // Start with two slashes: UNC filename starting with a server.
        parse_authority_split(parts, segments.at(2));

        // Remove the server-name and leading double slash. But keep a leading slash in,
        // because what follows is an absolute path.
        segments.erase(segments.begin() + 1, segments.begin() + 3);
    }

    // Extract optional drive from file path.
    if (segments.size() >= 2 && segments.at(0).size() == 0 && segments.at(1).find(':') != std::string_view::npos) {
        // Drive following a UNC/URL server/authority name (the server/authority may be empty)
        // First strip off the slash in front of the drive letter.
        segments.erase(segments.begin());

        ttlet i = segments.at(0).find(':');
        parts.drive = segments.at(0).substr(0, i);
        segments.at(0) = segments.at(0).substr(i + 1);

    } else if (segments.size() >= 1 && segments.at(0).find(':') != std::string_view::npos) {
        // A drive letter as the first segment of a path.
        ttlet i = segments.at(0).find(':');
        parts.drive = segments.at(0).substr(0, i);
        segments.at(0) = segments.at(0).substr(i + 1);
    }

    // Check for a leading slash '/' meaning an absolute path.
    parts.absolute = segments.size() >= 1 && segments.at(0).size() == 0;
    parts.segments = std::move(segments);
    normalize_url_parts(parts);
}

static void parse_path_split(url_parts &parts, std::string_view path, char sep='/') noexcept
{
    if (path.size() == 0) {
        // Empty path is relative.
        return parse_path_split(parts, std::vector<std::string_view>{});

    } else {
        return parse_path_split(parts, split(path, sep));
    }
}

static void parse_url_split(url_parts &parts, std::string_view url)
{
    // Find the scheme. A scheme must be at least two character
    // to differentiate it from a directory.
    for (size_t i = 0; i < url.size(); i++) {
        ttlet c = url.at(i);
        if (c == ':' && i >= 2) {
            parts.scheme = url.substr(0, i);
            url = url.substr(i + 1);
            break;

        } else if (!is_urlchar_scheme(c, i)) {
            // Not a scheme; wrong character or early ':'.
            break;
        }
    }

    // Find the fragment.
    ttlet fragment_i = url.rfind('#');
    if (fragment_i != url.npos) {
        parts.fragment = url.substr(fragment_i + 1);
        url = url.substr(0, fragment_i);
    }

    // Find the query.
    ttlet query_i = url.rfind('?');
    if (query_i != url.npos) {
        parts.query = url.substr(query_i + 1);
        url = url.substr(0, query_i);
    }

    parse_path_split(parts, url);
}

static size_t generate_size_guess(url_parts const &parts, bool only_path) noexcept
{
    ttlet path_size = parts.authority.size() + parts.drive.size() + parts.segments.size() + 10;

    ttlet start_size = only_path ?
        path_size :
        path_size + parts.scheme.size() + parts.query.size() + parts.fragment.size();

    return std::accumulate(parts.segments.begin(), parts.segments.end(), start_size, [](size_t a, auto b) {
        return a + b.size();
        });
}

static void generate_path_append(std::string &r, url_parts const &parts, char sep='/') noexcept
{
    if (parts.authority.size() > 0) {
        r.append(2, sep);
        r.append(parts.authority);
    }

    if (parts.drive.size() > 0) {
        if (parts.authority.size()) {
            r += sep;
        }
        r.append(parts.drive);
        r += ':';
    }

    if (parts.absolute) {
        r += sep;
    }

    r.append(join(parts.segments, std::string(1, sep)));
}

static void generate_url_append(std::string &r, url_parts const &parts) noexcept
{
    if (parts.scheme.size() > 0) {
        r.append(parts.scheme);
        r += ':';
    }

    generate_path_append(r, parts);

    if (parts.query.size() > 0) {
        r += '?';
        r.append(parts.query);
    }

    if (parts.fragment.size() > 0) {
        r += '#';
        r.append(parts.fragment);
    }
}

std::string generate_url(url_parts const &parts) noexcept
{
    std::string r;
    r.reserve(generate_size_guess(parts, false));
    generate_url_append(r, parts);
    return r;
}

std::string generate_path(url_parts const &parts, char sep) noexcept
{
    std::string r;
    r.reserve(generate_size_guess(parts, true));
    generate_path_append(r, parts, sep);
    // Generally '/' and '\' are not allowed to be in filenames, so we
    // can decode the full path in one go.
    return url_decode(r);
}

std::string generate_native_path(url_parts const &parts) noexcept
{
    return generate_path(parts, native_path_seperator);
}

url_parts parse_url(std::string_view url) noexcept
{
    url_parts parts;
    parse_url_split(parts, url);
    return parts;
}

url_parts parse_path(std::string_view path, std::string &encodedPath) noexcept
{
    url_parts parts;
    parts.scheme = "file"; // string_view of char[] literal has now ownership issues.

    // Detect path seperator.
    ttlet forward_count = std::count(path.begin(), path.end(), '/');
    ttlet backward_count = std::count(path.begin(), path.end(), '\\');

    encodedPath = (forward_count >= backward_count) ?
        url_encode_part(path, is_urlchar_pchar_forward) :
        url_encode_part(path, is_urlchar_pchar_backward);

    ttlet sep = (forward_count >= backward_count) ? '/' : '\\';

    // Parse the path.
    parse_path_split(parts, encodedPath, sep);
    return parts;
}


void normalize_url_parts(url_parts &parts) noexcept
{
    auto &segments = parts.segments;
    
    for (auto i = segments.begin(); i != segments.end();) {
        if ((*i).size() == 0 || *i == "." || (parts.absolute && i == segments.begin() && *i == "..")) {
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

        if (i != segments.end()) {
            i++;
        }
    }
}

std::string normalize_url(std::string_view url) noexcept
{
    // parse_url() implies normalize_url_parts().
    return generate_url(parse_url(url));
}

url_parts concatenate_url_parts(url_parts const &lhs, url_parts const &rhs) noexcept
{
    auto parts = lhs;

    if (rhs.absolute) {
        // Replace the segments.
        parts.segments = rhs.segments;
    } else {
        std::copy(rhs.segments.begin(), rhs.segments.end(), std::back_inserter(parts.segments));
    }

    // Normalize the path.
    normalize_url_parts(parts);
    return parts;
}

std::string concatenate_url(std::string_view const lhs, std::string_view const rhs) noexcept
{
    ttlet lhs_parts = parse_url(lhs);
    ttlet rhs_parts = parse_url(rhs);
    ttlet merged_parts = concatenate_url_parts(lhs_parts, rhs_parts);
    return generate_url(merged_parts);
}

std::string filename_from_path(std::string_view path) noexcept
{
    ttlet i_fwd = path.rfind('/');
    ttlet i_bwd = path.rfind('\\');

    if (i_fwd != path.npos && (i_bwd == path.npos || i_bwd < i_fwd)) {
        return std::string{path.substr(i_fwd + 1)};
    } else if (i_bwd != path.npos && (i_fwd == path.npos || i_fwd < i_bwd)) {
        return std::string{path.substr(i_bwd + 1)};
    } else {
        return std::string{path};
    }
}

}
