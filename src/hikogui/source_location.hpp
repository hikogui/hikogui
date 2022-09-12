// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

class source_location {
public:
    constexpr source_location() noexcept = default;
    constexpr source_location(
        uint_least32_t line,
        uint_least32_t column,
        char const *file_name,
        char const *function_name) noexcept :
        _line(line), _column(column), _file_name(file_name), _function_name(function_name)
    {
    }

    constexpr source_location(source_location const &) noexcept = default;
    constexpr source_location(source_location &&) noexcept = default;
    constexpr source_location &operator=(source_location const &) noexcept = default;
    constexpr source_location &operator=(source_location &&) noexcept = default;

    [[nodiscard]] constexpr uint_least32_t line() const noexcept
    {
        return _line;
    }

    [[nodiscard]] constexpr uint_least32_t column() const noexcept
    {
        return _column;
    }

    [[nodiscard]] constexpr char const *file_name() const noexcept
    {
        return _file_name;
    }

    [[nodiscard]] constexpr char const *function_name() const noexcept
    {
        return _function_name;
    }

private:
    uint_least32_t _line = 0;
    uint_least32_t _column = 0;
    char const *_file_name = nullptr;
    char const *_function_name = nullptr;
};

#define hi_source_location_current() hi::source_location(__LINE__, 0, __FILE__, __func__)

inline std::string to_string(source_location const &rhs) noexcept
{
    return std::format("{}:{}", rhs.file_name(), rhs.line());
}

} // namespace hi::inline v1
