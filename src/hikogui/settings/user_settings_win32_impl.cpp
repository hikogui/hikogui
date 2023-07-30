

#include "user_settings.hpp"
#include "registry_win32.hpp"
#include "../path/path.hpp"
#include "../macros.hpp"
#include <format>
#include <string>

namespace hi { inline namespace v1 {

[[nodiscard]] static std::string user_setting_registry_path()
{
    return std::format("Software\\{}\\{}", get_application_vendor(), get_application_name());
}

[[nodiscard]] std::optional<std::string> get_user_setting_string(std::string_view name)
{
    // First check the registry of the current-user.
    if (hilet value = registry_read<std::string>(registry_key::current_user, user_setting_registry_path(), name)) {
        return *value;
    }

    // Now check the registry for the local-machine.
    // These are settings that where made by the Administrator of the machine.
    if (hilet value = registry_read<std::string>(registry_key::local_machine, user_setting_registry_path(), name)) {
        return *value;
    }

    // There where no settings configured.
    return std::nullopt;
}

[[nodiscard]] std::optional<long long> get_user_setting_integral(std::string_view name)
{
    // First check the registry of the current-user.
    if (hilet value = registry_read<long long>(registry_key::current_user, user_setting_registry_path(), name)) {
        return *value;
    }

    // Now check the registry for the local-machine.
    // These are settings that where made by the Administrator of the machine.
    if (hilet value = registry_read<long long>(registry_key::local_machine, user_setting_registry_path(), name)) {
        return *value;
    }

    // There where no settings configured.
    return std::nullopt;
}

void set_user_setting(std::string_view name, std::string_view value)
{
    registry_write(registry_key::current_user, user_setting_registry_path(), name, value);
}

void set_user_setting(std::string_view name, long long value)
{
    registry_write(registry_key::current_user, user_setting_registry_path(), name, narrow_cast<uint32_t>(value));
}

void delete_user_setting(std::string_view name)
{
    registry_delete(registry_key::current_user, user_setting_registry_path(), name);
}

void delete_user_settings()
{
    registry_delete(registry_key::current_user, user_setting_registry_path());
}

}}

