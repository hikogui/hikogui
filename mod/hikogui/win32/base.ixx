
module;
#include "../macros.hpp"
#include "../win32_headers.hpp"

#include <system_error>
#include <utility>
#include <string>
#include <expected>
#include <vector>
#include <algorithm>

export module hikogui_win32_base;

export namespace hi { inline namespace v1 {

export enum class win32_error : uint32_t {
    success = ERROR_SUCCESS,
    file_not_found = ERROR_FILE_NOT_FOUND,
    more_data = ERROR_MORE_DATA,
    invalid_data = ERROR_INVALID_DATA,
};

}} // namespace hi::v1

export template<>
struct std::is_error_code_enum<hi::win32_error> : std::true_type {};

export namespace hi { inline namespace v1 {

export struct win32_error_category : std::error_category {
    char const *name() const noexcept override
    {
        return "win32";
    }

    std::string message(int code) const override;

    bool equivalent(int code, std::error_condition const & condition) const noexcept override
    {
        switch (static_cast<hi::win32_error>(code)) {
        case hi::win32_error::file_not_found:
            return condition == std::errc::no_such_file_or_directory;
        case hi::win32_error::more_data:
            return condition == std::errc::message_size;
        case hi::win32_error::invalid_data:
            return condition == std::errc::bad_message;
        default:
            return false;
        };
    }
};

auto global_win32_error_category = win32_error_category{};

export [[nodiscard]] std::error_code make_error_code(win32_error code) noexcept
{
    return {static_cast<int>(code), global_win32_error_category};
}

export [[nodiscard]] win32_error win32_GetLastError() noexcept
{
    return static_cast<win32_error>(::GetLastError());
}

/** Convert a win32-API compatible std::wstring to a multi-byte std::string.
 * 
 * @param s The wide string to convert
 * @param code_page The code-page to use for conversion
 * @param flags The flags to passing
 * @return multi-byte string.
 */
export [[nodiscard]] std::expected<std::string, win32_error> win32_WideCharToMultiByte(std::wstring_view s, unsigned int code_page = CP_UTF8, uint32_t flags = 0) noexcept
{
    if (s.empty()) {
        // WideCharToMultiByte() does not handle empty strings, if it can not also convert the null-character.
        return std::string{};
    }

    auto s_len = static_cast<int>(static_cast<unsigned int>(s.size()));
    auto r_len = ::WideCharToMultiByte(code_page, flags, s.data(), s_len, nullptr, 0, nullptr, nullptr);
    if (r_len == 0) {
        return std::unexpected{win32_GetLastError()};
    }

    auto r = std::string(static_cast<size_t>(static_cast<std::make_signed_t<size_t>>(r_len)), '\0');
    r.resize_and_overwrite(r_len, [&](char *p, size_t count) {
        return ::WideCharToMultiByte(code_page, flags, s.data(), s_len, p, static_cast<int>(count), nullptr, nullptr);
    });

    if (r.empty()) {
        return std::unexpected{win32_GetLastError()};
    }

    return r;
}

/** Convert a win32-API compatible std::wstring to a multi-byte std::string.
 * 
 * @param s The wide string to convert
 * @param code_page The code-page to use for conversion
 * @param flags The flags to passing
 * @return multi-byte string.
 */
export [[nodiscard]] std::expected<std::wstring, win32_error> win32_MultiByteToWideChar(std::string_view s, unsigned int code_page = CP_UTF8, uint32_t flags = 0) noexcept
{
    if (s.empty()) {
        // MultiByteToWideChar() does not handle empty strings, if it can not also convert the null-character.
        return std::wstring{};
    }

    auto s_len = static_cast<int>(static_cast<unsigned int>(s.size()));
    auto r_len = ::MultiByteToWideChar(code_page, flags, s.data(), s_len, nullptr, 0);
    if (r_len == 0) {
        return std::unexpected{win32_GetLastError()};
    }

    auto r = std::wstring{};
    r.resize_and_overwrite(r_len, [&](wchar_t *p, size_t count) {
        return ::MultiByteToWideChar(code_page, flags, s.data(), s_len, p, static_cast<int>(count));
    });

    if (r.empty()) {
        return std::unexpected{win32_GetLastError()};
    }

    return r;
}

/** Convert a win32 zero terminated list of zero terminated strings.
 * 
 * This function will treat the array as-if it is a list of zero terminated strings,
 * where the last string is a zero terminated empty string.
 * 
 * @param first A pointer to a buffer of a zero terminated list of zero terminated string.
 * @param last A pointer one beyond the buffer.
 * @return A vector of UTF-8 encoded strings, win32_error::invalid_data when the list is incorrectly terminated.
 */
export [[nodiscard]] std::expected<std::vector<std::string>, win32_error> win32_MultiSZToStringVector(wchar_t const *first, wchar_t const *last) noexcept
{
    auto r = std::vector<std::string>{};

    while (first != last) {
        auto it_zero = std::find(first, last, wchar_t{0});
        if (it_zero == last) {
            // No termination found.
            return std::unexpected{win32_error::invalid_data};
        }

        hilet ws = std::wstring_view{first, static_cast<std::size_t>(it_zero - first)};
        if (ws.empty()) {
            // The list is terminated with an empty string.
            break;
        }

        if (auto s = win32_WideCharToMultiByte(ws)) {
            r.push_back(*s);
        } else {
            return std::unexpected{s.error()};
        }

        // Continue after the zero terminator.
        first = it_zero + 1;
    }

    return r;
}

export [[nodiscard]] std::expected<std::string, win32_error> win32_FormatMessage(win32_error error_code) noexcept
{
    hilet error_code_ = static_cast<DWORD>(std::to_underlying(error_code));

    // FormatMessageW() is unable to tell what the buffer size should be.
    // But 64Kbyte is the largest buffer that one should pass.
    LPWSTR buffer = nullptr;
    hilet result = ::FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        error_code_,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&buffer),
        0,
        NULL);

    if (result == 0) {
        return std::unexpected(win32_GetLastError());
    }

    auto r = win32_WideCharToMultiByte(std::wstring_view{buffer, result});
    LocalFree(buffer);
    return r;
}

export std::string win32_error_category::message(int code) const
{
    if (auto msg = win32_FormatMessage(static_cast<win32_error>(code))) {
        return *msg;

    } else {
        throw std::system_error(msg.error());
    }
}

}} // namespace hi::v1
