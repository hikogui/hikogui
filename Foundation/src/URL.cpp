// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/url_parser.hpp"
#include "TTauri/Foundation/glob.hpp"
#include <regex>

namespace TTauri {

URL::URL(std::string_view url) :
    value(normalize_url(url))
{
}

URL::URL(char const *url) :
    value(normalize_url(url))
{
}

URL::URL(std::string const &url) :
    value(normalize_url(url))
{
}

URL::URL(url_parts const &parts) :
    value(generate_url(parts))
{
}

size_t URL::hash() const noexcept
{
    return std::hash<std::string>{}(value);
}

std::string URL::string() const noexcept
{
    return value;
}

std::string_view URL::scheme() const noexcept
{
    return parse_url(value).scheme;
}

std::string URL::query() const noexcept
{
    return url_decode(parse_url(value).query, true);
}

std::string URL::fragment() const noexcept
{
    return url_decode(parse_url(value).fragment);
}

std::string URL::filename() const noexcept
{
    let parts = parse_url(value);
    if (parts.segments.size() > 0) {
        return url_decode(parts.segments.back());
    } else {
        return {};
    }
}

std::string URL::directory() const noexcept
{
    auto parts = parse_url(value);
    if (parts.segments.size() > 0) {
        parts.segments.pop_back();
    }
    return generate_path(parts);
}

std::string URL::nativeDirectory() const noexcept
{
    auto parts = parse_url(value);
    if (parts.segments.size() > 0) {
        parts.segments.pop_back();
    }
    return generate_native_path(parts);
}

std::string URL::extension() const noexcept
{
    let fn = filename();
    let i = fn.rfind('.');
    return fn.substr((i != fn.npos) ? (i + 1) : fn.size());
}

std::vector<std::string> URL::pathSegments() const noexcept
{
    let parts = parse_url(value);
    return transform<std::vector<std::string>>(parts.segments, [](auto x) {
        return url_decode(x);
        });
}

std::string URL::path() const noexcept
{
    return generate_path(parse_url(value));
}

std::string URL::nativePath() const noexcept
{
    return generate_native_path(parse_url(value));
}

std::wstring URL::nativeWPath() const noexcept
{
    return translateString<std::wstring>(nativePath());
}

bool URL::isAbsolute() const noexcept
{
    return parse_url(value).absolute;
}

bool URL::isRelative() const noexcept
{
    return !isAbsolute();
}

URL URL::urlByAppendingPath(URL const &other) const noexcept
{
    let this_parts = parse_url(value);
    let other_parts = parse_url(other.value);
    let new_parts = concatenate_url_parts(this_parts, other_parts);
    return URL(new_parts);
}

URL URL::urlByAppendingPath(std::string_view const other) const noexcept
{
    return urlByAppendingPath(URL::urlFromPath(other));
}

URL URL::urlByAppendingPath(std::string const &other) const noexcept
{
    return urlByAppendingPath(URL::urlFromPath(other));
}

URL URL::urlByAppendingPath(char const *other) const noexcept
{
    return urlByAppendingPath(URL::urlFromPath(other));
}

URL URL::urlByAppendingPath(std::wstring_view const other) const noexcept
{
    return urlByAppendingPath(URL::urlFromWPath(other));
}

URL URL::urlByAppendingPath(std::wstring const &other) const noexcept
{
    return urlByAppendingPath(URL::urlFromWPath(other));
}

URL URL::urlByAppendingPath(wchar_t const *other) const noexcept
{
    return urlByAppendingPath(URL::urlFromWPath(other));
}

URL URL::urlByRemovingFilename() const noexcept
{
    auto parts = parse_url(value);
    if (parts.segments.size() > 0) {
        parts.segments.pop_back();
    }
    return URL(parts);
}

URL URL::urlByRemovingBaseURL(URL const &url) const noexcept
{

}

static bool urlMatchGlob(URL url, URL glob, bool exactMatch) noexcept
{
    let urlSegments = url.pathSegments();
    let globSegments = glob.pathSegments();

    size_t globIndex = 0;
    for (; globIndex < globSegments.size(); globIndex++) { 

    }

    return !exactMatch || globIndex == globSegments.size();
}

static void urlsByRecursiveScanning(std::string base, glob_token_list_t const &glob, std::vector<URL> &result) noexcept
{
    for (let &filename: URL::filenamesByScanningDirectory(base)) {
        if (filename.back() == '/') {
            let directory = std::string_view(filename.data(), filename.size() - 1);
            auto recursePath = base + "/";
            recursePath += directory;

            if (matchGlob(glob, recursePath) != glob_match_result_t::No) {
                urlsByRecursiveScanning(recursePath, glob, result);
            }

        } else {
            let finalPath = base + '/' + filename;
            if (matchGlob(glob, finalPath) != glob_match_result_t::Match) {
                result.push_back(URL::urlFromPath(finalPath));
            }
        }
    }
}

std::vector<URL> URL::urlsByScanningWithGlobPattern() const noexcept
{
    let glob = parseGlob(path());
    let basePath = basePathOfGlob(glob);

    std::vector<URL> urls;
    urlsByRecursiveScanning(basePath, glob, urls);
    return urls;
}

URL URL::urlFromPath(std::string_view const path) noexcept
{
    std::string tmp;
    let parts = parse_path(path, tmp);
    return URL(parts);
}

URL URL::urlFromWPath(std::wstring_view const path) noexcept
{
    return urlFromPath(translateString<std::string>(path));
}

URL URL::urlFromExecutableDirectory() noexcept
{
    static auto r = urlFromExecutableFile().urlByRemovingFilename();
    return r;
}

URL URL::urlFromApplicationLogDirectory() noexcept
{
    return urlFromApplicationDataDirectory() / "Log";
}

std::ostream& operator<<(std::ostream& lhs, const URL& rhs)
{
    lhs << rhs.string();
    return lhs;
}

std::string URL::nativePathFromPath(std::string_view path) noexcept
{
    std::string r = static_cast<std::string>(path);

    for (auto &c: r) {
        if (c == '/') {
            c = native_path_seperator;
        }
    }
    return r;
}

std::wstring URL::nativeWPathFromPath(std::string_view path) noexcept
{
    return translateString<std::wstring>(nativePathFromPath(path));
}

}
