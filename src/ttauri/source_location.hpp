

#pragma once

namespace tt {

class source_location {
public:
    constexpr source_location(uint_least32_t line, uint_least32_t column, char const *file_name, char const *function_name) noexcept :
        _line(line), _column(column), _file_name(file_name), _function_name(function_name) {}

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
    uint_least32_t _line;
    uint_least32_t _column;
    char const *_file_name;
    char const *_function_name;
};

#define tt_source_location() tt::source_location(__LINE__, 0, __FILE__, __func__)

}

