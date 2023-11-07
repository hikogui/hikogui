

#pragma once

#include "enum_metadata.hpp"
#include "../macros.hpp"
#include <format>

hi_export_module(hikogui.utility.policy);

hi_export namespace hi { inline namespace v1 {

/** The performance policy to use.
 *
 * This policy is used as an argument when initializing a system to
 * select the correct parameters to satisfy the given policy.
 */
enum class policy { unspecified, low_power, high_performance };

// clang-format off
constexpr auto policy_metadata = enum_metadata{
    policy::unspecified, "unspecified",
    policy::low_power, "low-power",
    policy::high_performance, "high-performance"
};
// clang-format on

}} // namespace hi::v1

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::policy, char> : std::formatter<std::string_view, char> {
    auto format(hi::policy const& t, auto& fc) const
    {
        return std::formatter<std::string_view, char>::format(hi::policy_metadata[t], fc);
    }
};
