// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef ERROR_INFO_HPP
#define ERROR_INFO_HPP

#pragma once

#include "source_location.hpp"
#include "tag.hpp"
#include "concepts.hpp"
#include "check.hpp"
#include <cstdint>
#include <string>
#include <type_traits>
#include <optional>

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

namespace detail {

struct error_info_entry_base {
    uint64_t version;

    /** Get a string representation of the tag and value of this entry.
     */
    [[nodiscard]] virtual std::string string() noexcept = 0;
};

template<typename Tag>
struct error_info_entry : public error_info_entry_base {
    using value_type = typename Tag::value_type;

    std::optional<value_type> value;

    [[nodiscard]] std::string string() noexcept override
    {
        using std::to_string;
        using tt::to_string;

        if constexpr (to_stringable<value_type>) {
            return fmt::format("{}={}", tag_name<Tag>(), to_string(*value));
        } else {
            return fmt::format("{}={}", tag_name<Tag>(), *value);
        }
    }
};

}

/** Error information passed alongside an error code or exception.
 *
 * Example:
 * ```
 * try {
 *     try {
 *         tt_error_info().set<errno_tag>(errno);
 *         throw std::runtime_error("foo");
 *
 *     } catch (runtime_error const &e) {
 *         // Add more information during the catch.
 *         tt::error_info(true).set<url_tag>(file_url);
 *         throw
 *     }
 * } catch (runtime_error const &e) {
 *     auto error_url = *tt::error_info::pop<url_tag>();
 *     auto error_errno = *tt::error_info::pop<errno_tag>();
 *     tt_log_error("Config file error in file {}, errno={}", error_url, error_errno);
 * }
 * ```
 */
class error_info {
public:
    /** Open an error info transaction.
     * A transaction may only be opened when the error_info is
     * in idle state; just before throwing an exception.
     */
    error_info(source_location location) noexcept
    {
        ++version;
        tt_axiom(state == state::closed);
        tt_axiom(version != 0);

        state = state::writing;
        set<source_location_tag>(location);
    }

    /** Reopen a error info transaction.
     * A closed transaction may be reopened to add more information
     * to a thrown exception. Used when (re-)throwing an exception from a
     * catch block.
     * 
     * @param reopen True when reopening an error_info transaction. Must be set to true.
     */
    error_info(bool reopen) noexcept
    {
        tt_axiom(reopen == true);
        if (state == state::closed) {
            // Start a new transaction if it wasn't already opened.
            // This may happen on a catch statement from a traditional exception.
            ++version;

        } else {
            tt_axiom(state == state::reading);
        }

        tt_axiom(version != 0);

        state = state::writing;
    }

    ~error_info()
    {
        tt_axiom(state == state::writing);
        tt_axiom(version != 0);

        state = state::reading;
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
    error_info &set(Arg &&value) noexcept
    {
        tt_axiom(state == state::writing);
        tt_axiom(version != 0);

        auto &e = entry<Tag>;
        if (e.version == 0) {
            // Track this entry from now on, so that information on all error_info
            // can be listed dynamically.
            [[unlikely]] register_entry(e);
        }

        e.version = version;
        e.value = std::forward<Arg>(value);
        return *this;
    }

    /** Close the current transaction.
     * After the transaction is closed it can no longer be reopened or written to.
     * 
     * This function should be called in a non-throwing catch block, either directly
     * or through the usage of `error_info::pop` or `to_string(std::exception &e, bool)`
     */
    static void close() noexcept
    {
        tt_axiom(state == state::closed || state == state::reading);

        state = state::closed;
    }

    /** Destructive read data from the current transaction.
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
    static std::optional<typename Tag::value_type> pop() noexcept
    {
        close();
        tt_axiom(state == state::closed);

        auto &e = entry<Tag>;
        if (version != 0 && e.version == version) {
            return std::exchange(e.value, {});
        } else {
            return {};
        }
    }

    /** Non-destructive read data from the current transaction.
     * This function will NOT close the current transaction.
     *
     * @tparam Tag a struct type used as an identifier.
     *             This struct type should have a public typedef of `value_type`
     *             for the value returned by this function.
     * @return The value to if it was set, or empty.
     */
    template<typename Tag>
    static std::optional<typename Tag::value_type> peek() noexcept
    {
        auto &e = entry<Tag>;
        if (version != 0 && e.version == version) {
            return e.value;
        } else {
            return {};
        }
    }

    /** Return the list of entries with their current set value.
     */
    static std::string string() noexcept
    {
        std::string r;

        for (auto e: entries) {
            if (version != 0 && e->version == version) {
                if (!r.empty()) {
                    r += ", ";
                }

                r += e->string();
            }
        }
        return r;
    }

private:
    enum class state {
        closed, ///< No longer possible to add error_information. Reading is still possible.
        writing, ///< Currently adding error information using `error_info::set()`.
        reading ///< Waiting to reopen the transaction, or closing due to reading information.
    };

    /** The version that was last committed.
     */
    inline static thread_local uint64_t version;

    /** Current state the error_info is in.
     */
    inline static thread_local state state;

    /** The last value that was committed.
     */
    template<typename Tag>
    inline static thread_local detail::error_info_entry<Tag> entry;

    /** A list of entries to check when listing all error_info of an exception.
     * When an entry is set for the first time, it will be added to this list.
     */
    inline static thread_local std::vector<detail::error_info_entry_base *> entries;

    tt_no_inline static void register_entry(detail::error_info_entry_base &e) noexcept
    {
        entries.push_back(&e);
    }
};

/** Create a message including the exception and any error information.
 * This function will close the current transaction.
 */
[[nodiscard]] inline std::string to_string(std::exception const &e, bool close=true) noexcept
{
    if (close) {
        error_info::close();
    }
    return fmt::format("{}: {}: {}", typeid(e).name(), e.what(), error_info::string());
}

} // namespace tt

#define tt_error_info() tt::error_info(tt_source_location_current())

#endif
