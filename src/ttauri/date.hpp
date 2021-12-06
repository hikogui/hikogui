// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "assert.hpp"
#include <date/date.h>
#include <format>
#include <string>
#include <compare>

namespace tt::inline v1 {

[[nodiscard]] std::string to_string(std::chrono::year y) noexcept
{
    return std::format("{}", static_cast<int>(y));
}

class quarter {
    unsigned int q;

public:
    explicit constexpr quarter(std::chrono::month m) noexcept : q(((static_cast<unsigned>(m) - 1) / 3) + 1) {}

    [[nodiscard]] explicit constexpr operator unsigned() const noexcept
    {
        return q;
    }

    [[nodiscard]] constexpr std::chrono::month first_month() const noexcept
    {
        return std::chrono::month{((q - 1) * 3) + 1};
    }

    [[nodiscard]] constexpr std::chrono::month last_month() const noexcept
    {
        return first_month() + std::chrono::months{2};
    }

    [[nodiscard]] constexpr std::chrono::month_day first() const noexcept
    {
        return std::chrono::month_day{first_month(), std::day{1}};
    }

    [[nodiscard]] constexpr std::chrono::month_day last() const noexcept
    {
        switch (q) {
        case 1: return std::chrono::month_day{last_month(), std::day{31}};
        case 2: return std::chrono::month_day{last_month(), std::day{30}};
        case 3: return std::chrono::month_day{last_month(), std::day{30}};
        case 4: return std::chrono::month_day{last_month(), std::day{31}};
        default: tt_no_default();
        }
    }

    [[nodiscard]] constexpr bool increment_carry() noexcept
    {
        if (++q > 4) {
            q = 1;
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool contains(std::chrono::month_day const &md) const noexcept
    {
        return md >= first() && md <= last();
    }

    [[nodiscard]] friend constexpr bool operator==(quarter const &lhs, quarter const &rhs) noexcept
    {
        return lhs.q == rhs.q;
    }

    [[nodiscard]] friend constexpr auto operator<=>(quarter const &lhs, quarter const &rhs) noexcept
    {
        return lhs.q <=> rhs.q;
    }

    [[nodiscard]] friend std::string to_string(quarter const &rhs) noexcept
    {
        return std::format("{}", rhs.q);
    }

    friend std::ostream &operator<<(std::ostream &lhs, quarter const &rhs) noexcept
    {
        return lhs << to_string(rhs);
    }
};

class year_quarter {
    std::chrono::year y;
    quarter q;

public:
    explicit constexpr year_quarter(std::chrono::year_month const &ym) : y(ym.year()), q(ym.month()) {}

    explicit constexpr year_quarter(std::chrono::year_month_day const &ymd) :
        year_quarter(std::chrono::year_month{ymd.year(), ymd.month()})
    {
    }

    constexpr year_quarter &operator++() noexcept
    {
        if (q.increment_carry()) {
            ++y;
        }
        return *this;
    }

    [[nodiscard]] constexpr std::chrono::year_month first_year_month() noexcept
    {
        return std::chrono::year_month{y, q.first_month()};
    }

    [[nodiscard]] constexpr std::chrono::year_month last_year_month() noexcept
    {
        return std::chrono::year_month{y, q.last_month()};
    }

    [[nodiscard]] constexpr std::chrono::year_month_day first() noexcept
    {
        ttlet md = q.first();
        return std::chrono::year_month_day{y, md.month(), md.day()};
    }

    [[nodiscard]] constexpr std::chrono::year_month_day last() noexcept
    {
        ttlet md = q.last();
        return std::chrono::year_month_day{y, md.month(), md.day()};
    }

    [[nodiscard]] bool contains(std::chrono::year_month_day const &ymd) const noexcept
    {
        return y == ymd.year() && q.contains(std::chrono::month_day(ymd.month(), ymd.day()));
    }

    [[nodiscard]] friend constexpr bool operator==(year_quarter const &lhs, year_quarter const &rhs) noexcept = default;
    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(year_quarter const &lhs, year_quarter const &rhs) noexcept = default;


    [[nodiscard]] friend std::string to_string(year_quarter const &rhs) noexcept
    {
        return std::format("{}Q{}", rhs.y, rhs.q);
    }

    friend std::ostream &operator<<(std::ostream &lhs, year_quarter const &rhs) noexcept
    {
        return lhs << to_string(rhs);
    }
};

} // namespace tt::inline v1