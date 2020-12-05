
#pragma once

#include "source_location.hpp"
#include "utils.hpp"
#include <cstdint>
#include <string>

namespace tt {

/** Location in the source file where an error was thrown.
 */
struct source_location_tag {
    using value_type = source_location;
};

/** Used to define for which URL a io_error was thrown.
 */
class URL;
struct url_tag {
    using value_type = URL;
};

struct key_tag {
    using value_type = std::string;
};

struct error_message_tag {
    using value_type = std::string;
};

/** Used to define the location in a text file where there was an error during parsing.
 */
class parse_location;
struct parse_location_tag {
    using value_type = parse_location;
};

/** Error information passed alongside an error code or exception.
 *
 * Example:
 * ```
 * try {
 *     try {
 *         tt::error_info().set<errno_tag>(errno);
 *         throw std::runtime_error("foo");
 *
 *     } catch (runtime_error &e) {
 *         // Add more information during the catch.
 *         tt::error_info().copy<errno_tag>().set<url_tag>(file_url);
 *         throw
 *     }
 * } catch (runtime_error &e) {
 *     auto error_url = *tt::error_info::get<url_tag>();
 *     auto error_errno = *tt::error_info::get<errno_tag>();
 *     LOG_ERROR("Config file error in file {}, errno={}", error_url, error_errno);
 * }
 * ```
 */
class error_info {
public:
    /** Open an error info transaction.
     *
     * If the transaction is already opened, such as during a rethrow,
     * the reuses the current opened transaction.
     *
     * If a new transaction is opened the source_location is recorded.
     *
     */
    error_info(source_location location) noexcept
    {
        if ((error_info::last_version & 2 == 0) {
            // Open a new transaction.
            this->version = ++error_info::last_version + 1;
            this->set<source_location_tag>(location);
        } else {
            this->version = error_info::last_version + 1;
        }
    }

    error_info(error_info &&rhs) noexcept = delete;
    error_info &operator=(error_info &&rhs) noexcept = delete;
    error_info(error_info const &rhs) = delete;
    error_info &operator=(error_info const &rhs) = delete;

    /** Set an information for a given tag.
     * You can chain multiple .set() calls.
     *
     * @tparam Tag a struct type used as an identifier.
     *             This struct type should have a public typedef of `value_type`
     *             for the value set in this function.
     * @param value The value to set.
     * @return A reference to the error_info, for chaining of .set() calls.
     */
    template<typename Tag, typename Arg>
    error_info &set(Arg &&value) noexcept {
        auto &entry = entry<Tag>;

        if (entry.version == 0) {
            entries.push_back(&entry);
        }

        entry.version = this->version;
        entry.value = std::forward<Arg>(value);
        return *this;
    }

    static void close() noexcept
    {
        if ((error_info::last_version % 2) == 1) {
            ++error_info::last_version;
        }
    }

    /** Read data from the current transaction.
     * This function will close the current transaction.
     * Continued `pop()` and `peek()` calls will still be able to read from
     * the current, but closed, transaction.
     *
     * @tparam Tag a struct type used as an identifier.
     *             This struct type should have a public typedef of `value_type`
     *             for the value returned by this function.
     * @return The value to if it was set, or empty.
     */
    template<typename Tag>
    static std::optional<typename Tag::value_type> pop() noexcept {
        close();

        auto &entry = error_info::entry<Tag>;
        if (entry.version == error_info::last_version) {
            return std::exchange(error_info::value<Tag,Tag::value_type>, {});
        } else {
            return {};
        }
    }

    /** Read data from the current transaction.
     * This function will NOT close the current transaction.
     *
     * @tparam Tag a struct type used as an identifier.
     *             This struct type should have a public typedef of `value_type`
     *             for the value returned by this function.
     * @return The value to if it was set, or empty.
     */
    template<typename Tag>
    static std::optional<typename Tag::value_type> peek() noexcept {
        auto &entry = error_info::entry<Tag>;
        if (entry.version == error_info::last_version + 1) {
            return *error_info::value<Tag,Tag::value_type>;
        } else {
            return {};
        }
    }

private:
    struct entry_base {
        uint64_t version;

        [[nodiscard]] virtual std::string string() noexcept = 0;
    };

    template<typename T>
    struct entry : public entry_base {
        std::optional<T> value;

        [[nodiscard]] std::string string() noexcept override
        {
            return to_string(*value);
        }
    };

    /** Version of the current transaction.
     * This must match last_version during the transaction.
     */
    uint64_t version;

    /** The version that was last committed.
     */
    inline static thread_local uint64_t last_version;

    /** The last value that was committed.
     */
    template<typename Tag>
    inline static thread_local entry<typename Tag::value_type> entry;

    /** A list of entries to check when listing all error_info of an exception.
     * When an entry is set for the first time, it will be added to this list.
     */
    inline static thread_local std::vector<entry_base *> entries;
};

/** Create a message including the exception and any error information.
 * This function will close the current transaction.
 */
[[nodiscard]] inline std::string to_string(std::exception const &e) noexcept
{
    error_info::close();
    return e.what();
}

}

#define tt_error_info() tt::error_info(tt_source_location())

