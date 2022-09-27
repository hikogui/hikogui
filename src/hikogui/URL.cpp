// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "URL.hpp"
#include "strings.hpp"
#include "utility.hpp"
#include "url_parser.hpp"
#include "file_view.hpp"
#include "exception.hpp"
#include "static_resource_view.hpp"
#include "log.hpp"
#include <regex>

namespace hi::inline v1 {


URL URL::url_from_current_working_directory() noexcept
{
    return URL(std::filesystem::current_path());
}

URL URL::url_from_executable_directory() noexcept
{
    auto r = url_from_executable_file();
    r.remove_filename();
    return r;
}

URL URL::url_from_application_log_directory() noexcept
{
    return url_from_application_data_directory() / "Log";
}

std::unique_ptr<resource_view> URL::loadView() const
{
    if (scheme() == "resource") {
        try {
            if (auto filename_ = filename()) {
                auto view = static_resource_view::loadView(*filename_);
                hi_log_info("Loaded resource {} from executable.", *this);
                return view;
            } else {
                throw url_error(std::format("Missing filename on resource: url '{}'", *this));
            }

        } catch (key_error const&) {
            hilet absolute_location = URL::url_from_resource_directory() / *this;
            auto view = std::make_unique<file_view>(absolute_location);
            hi_log_info("Loaded resource {} from filesystem at {}.", *this, absolute_location);
            return view;
        }

    } else if (not scheme() or scheme() == "file") {
        auto view = std::make_unique<file_view>(*this);
        hi_log_info("Loaded resource {} from filesystem.", *this);
        return view;

    } else {
        throw url_error(std::format("{}: Unknown scheme for loading a resource", *this));
    }
}

} // namespace hi::inline v1
