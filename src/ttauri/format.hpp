// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace tt {
namespace detail {
template<typename Arg>
auto u8format_argument_cast(Arg const &arg) noexcept -> std::
    conditional_t<std::is_same_v<Arg, std::u8string_view> || std::is_same_v<Arg, std::u8string>, std::string_view, Arg const &>
{
    if constexpr (std::is_same_v<Arg, std::u8string_view> || std::is_same_v<Arg, std::u8string>) {
        return std::string_view{reinterpret_cast<char const *>(arg.data()), arg.size()};
    } else {
        return arg;
    }
}

} // namespace tt

template<typename... Args>
std::u8string format(const std::locale &loc, std::u8string_view fmt, const Args &... args)
{
    // The current implementation assumes that `std::format()` is 8-bit clean and therefor compatible with UTF-8.
    auto r = fmt::format(detail::u8format_argument_cast(fmt), detail::u8format_argument_cast(args)...);

    // The following will need to allocate a copy of the formatted string.
    // XXX sanitize the UTF-8 string here.
    return std::u8string(reinterpret_cast<char8_t *>(r.data()), r.size());
}

template<typename... Args>
std::u8string format(std::u8string_view fmt, const Args &... args)
{
    return format(std::locale::classic(), fmt, args...);
}

constexpr inline bool format_uses_arg_ids(const char *fmt)
{
    bool start_placeholder = false;

    while (true) {
        ttlet c = *(fmt++);
        if (c == 0) {
            return false;
        } else if (c == '{') {
            start_placeholder = true;
        } else if (start_placeholder && c >= '0' && c <= '9') {
            return true;
        } else {
            start_placeholder = false;
        }
    }
}

}