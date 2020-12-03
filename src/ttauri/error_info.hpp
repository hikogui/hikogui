
#pragma once

#include "source_location.hpp"
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
    /** Open an error info capture transaction.
     * This will implicitly set the source_location_tag information.
     */
    error_info(source_location location) noexcept :
        version(++error_info::last_version)
    {
        tt_assume(error_info::last_version == this->version);
        this->set<source_location_tag>(location);
    }

    error_info(error_info &&rhs) noexcept : version(rhs.version)
    {
        tt_assume(error_info::last_version == this->version);
        rhs.version = 0;
    }

    error_info &operator=(error_info &&rhs) noexcept
    {
        tt_assume(this != &rhs);
        this->version = rhs.version;
        tt_assume(error_info::last_version == this->version);
        rhs.version = 0;
    }

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
        tt_assume(error_info::last_version == this->version);

        error_info::value_version<Tag> = this->version;
        error_info::value<Tag,typename Tag::value_type> = std::forward<Arg>(value);
        return *this;
    }

    /** Copy a value from the previous commit into the current error_info.
     * The copy of a value is only done if it was committed by the previous
     * error_info transaction.
     *
     * @tparam Tag a struct type used as an identifier.
     *             This struct type should have a public typedef of `value_type`
     *             for the value set in this function.
     * @return A reference to the error_info, for chaining of .set() calls.
     */
    template<typename Tag>
    error_info &copy() const noexcept {
        tt_assume(error_info::last_version == this->version);

        if (error_info::value_version<Tag> == this->version - 1) {
            error_info::value_version<Tag> = this->version;
        }
        return *this;
    }

    /** Read data from the previous commit.
     *
     * @tparam Tag a struct type used as an identifier.
     *             This struct type should have a public typedef of `value_type`
     *             for the value returned by this function.
     * @return The value to if it was set, or empty.
     */
    template<typename Tag>
    static std::optional<typename Tag::value_type> get() noexcept {
        if (error_info::value_version<Tag> == error_info::last_version) {
            return *error_info::value<Tag,Tag::value_type>;
        } else {
            return {};
        }
    }

private:
    /** Version of the current transaction.
     * This must match last_version during the transaction.
     */
    uint64_t version;

    /** The version that was last committed.
     */
    inline static thread_local uint64_t last_version;

    /** The version where a value was committed.
     */
    template<typename Tag>
    inline static thread_local uint64_t value_version;

    /** The last value that was committed.
     */
    template<typename Tag, typename T>
    inline static thread_local std::optional<T> value;
};

/** Create a message including the exception and any error information.
 */
[[nodiscard]] inline std::string to_string(std::exception const &e) noexcept
{
    return e.what();
}

}

#define tt_error_info() tt::error_info(tt_source_location())

