// Copyright 2019 Pokitec
// All rights reserved.

#include "URLPath.hpp"
#include "URL.hpp"
#include "strings.hpp"
#include "utils.hpp"
#include <filesystem>

namespace TTauri {

URLPath::URLPath(std::string const &path)
{
    absolute = (path.front() == '/');

    let encodedSegments = split(path.substr(absolute ? 1 : 0), '/');
    segments = transform<std::vector<std::string>>(encodedSegments, [](auto x) {
        return url_decode(x);
        });
}

URLPath::URLPath(boost::filesystem::path const &path)
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

URLPath URLPath::urlPathFromWin32Path(std::wstring_view const &path_wstring)
{
    let path_u8string = translateString<std::string>(path_wstring);
    let path = boost::filesystem::path(path_u8string);
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

}