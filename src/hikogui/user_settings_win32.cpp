

#include "user_settings.hpp"

namespace hi { inline namespace v1 {


[[nodiscard]] std::optional<std::string> get_user_setting_string(std::string_view key);
[[nodiscard]] std::optional<long long> get_user_setting_integral(std::string_view key);

void set_user_setting(std::string_view key, std::string_view value);


void set_user_setting(std::string_view key, long long value);

void delete_user_setting(std::string_view key);

[[nodiscard]] std::vector<std::string> list_user_settings() noexcept;

}}

