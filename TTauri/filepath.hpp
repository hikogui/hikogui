

#pragma once

namespace TTauri {

struct filepath_parts {
    std::string_view server;
    std::string_view drive;
    bool absolute;
    std::vector<std::string_view> segments;
};

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
filepath_parts parse_path(std::vector<std::string_view> segments) noexcept
{
    filepath_parts parts;

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

filepath_parts parse_path(std::vector<std::string_view> segments) noexcept
{
    return parse_path(split(path, '/', '\\'));
}

std::string generate_path(filepath_parts const &parts) noexcept
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

}
