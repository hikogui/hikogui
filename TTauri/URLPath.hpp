// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "URLAuthority.hpp"
#include "URLPath.hpp"
#include "required.hpp"
#include <boost/filesystem.hpp>
#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <numeric>

namespace boost::filesystem {
    class path;
}

namespace TTauri {

struct URLPath {
    bool absolute = false;
    std::vector<std::string> segments = {};

    URLPath() = default;
    URLPath(std::string const &path);
    URLPath(boost::filesystem::path const &path);

    std::string path_string() const;

    std::string const &filename() const;

    std::string extension() const;

    static URLPath urlPathFromWin32Path(std::wstring_view const &path_wstring);
};

std::string to_string(URLPath const &path);
size_t file_size(URLPath const &path);

bool operator==(URLPath const &lhs, URLPath const &rhs);
bool operator<(URLPath const &lhs, URLPath const &rhs);

}

namespace std {

template<>
struct hash<TTauri::URLPath> {
    typedef TTauri::URLPath argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& path) const noexcept {
        auto h = std::accumulate(path.segments.begin(), path.segments.end(), static_cast<size_t>(0), [](auto a, auto x) {
            return a ^ std::hash<decltype(x)>{}(x);
            });

        return h ^ std::hash<decltype(path.absolute)>{}(path.absolute);
    }
};

}
