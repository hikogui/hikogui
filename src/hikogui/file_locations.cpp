

#include "file_locations.hpp"

namespace hi { inline namespace v1 {


[[nodiscard]] std::filesystem::path url_from_executable_directory() noexcept
{
    auto r = executable_path();
    r.remove_filename();
    return r;
}

[[nodiscard]] std::filesystem::path application_log_directory() noexcept
{
    return application_data_directory() / "Log";
}


}}

