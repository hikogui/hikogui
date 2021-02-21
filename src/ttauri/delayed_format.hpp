// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <fmt/format.h>
#include <tuple>
#include "forward_value.hpp"
#include "fixed_string.hpp"

#pragma once

namespace tt {

/** Delayed formatting.
 * This class will capture all the arguments so that it may be passed
 * to another thread. Then call the function operator to do the actual formatting.
 */
template<basic_fixed_string Fmt, typename... Values>
class delayed_format {
public:
    static_assert(std::is_same_v<decltype(Fmt)::value_type, char>, "Fmt must be a basic_fixed_string<char>");

    delayed_format(delayed_format &&) noexcept = default;
    delayed_format(delayed_format const &) noexcept = default;
    delayed_format &operator=(delayed_format &&) noexcept = default;
    delayed_format &operator=(delayed_format const &) noexcept = default;

    /** Construct a delayed format.
     *
     * All arguments are passed by forwarding-references so that values can be
     * moved into the storage of the delayed_format.
     *
     * Arguments passed by reference will be copied. Arguments passed by std::string_view
     * or std::span will be copied into a std::string or std::vector.
     *
     * Literal strings will not be copied, instead a pointer is taken.
     *
     * @param args The parameters to std::format, including the fmt paramater,
     *             but excluding the locale.
     */
    template<typename... Args>
    delayed_format(Args const &...args) noexcept : _values(args...)
    {
    }

    /** Format now.
     * @return The formatted string.
     */
    [[nodiscard]] std::string operator()() const noexcept
    {
        auto tmp = std::tuple_cat(std::tuple{static_cast<char const *>(Fmt)}, _values);
        return std::apply(fmt::format<char const *,Values const &...>, std::move(tmp));
    }

    /** Format now.
     * @param loc The locale to use for formatting.
     * @return The formatted string.
     */
    [[nodiscard]] std::string operator()(std::locale const &loc) const noexcept
    {
        tt_not_implemented();
    }

private:
    std::tuple<Values...> _values;
};

template<fixed_string Fmt, typename... Args>
delayed_format(Args &&...) -> delayed_format<Fmt, forward_value_t<Args>...>;

} // namespace tt
