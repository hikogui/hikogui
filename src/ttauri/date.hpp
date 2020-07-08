// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "assert.hpp"
#include <date/date.h>
#include <fmt/format.h>
#include <string>

namespace tt {

[[nodiscard]] std::string to_string(date::year y) noexcept {
    return fmt::format("{}", static_cast<int>(y));
}

class quarter {
    unsigned int q;

public:
    explicit constexpr quarter(date::month m) noexcept :
        q(((static_cast<unsigned>(m) - 1) / 3) + 1) {}

    [[nodiscard]] explicit constexpr operator unsigned () const noexcept {
        return q;
    }

    [[nodiscard]] constexpr date::month first_month() const noexcept {
        return date::month{((q-1) * 3) + 1};
    }

    [[nodiscard]] constexpr date::month last_month() const noexcept {
        return first_month() + date::months{2};
    }

    [[nodiscard]] constexpr date::month_day first() const noexcept {
        return date::month_day{first_month(), date::day{1}};
    }

    [[nodiscard]] constexpr date::month_day last() const noexcept {
        switch (q) {
        case 1: return date::month_day{last_month(), date::day{31}};
        case 2: return date::month_day{last_month(), date::day{30}};
        case 3: return date::month_day{last_month(), date::day{30}};
        case 4: return date::month_day{last_month(), date::day{31}};
        default: tt_no_default;
        }
    }

    [[nodiscard]] constexpr bool increment_carry() noexcept {
        if (++q > 4) {
            q = 1;
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] bool contains(date::month_day const &md) const noexcept {
        return md >= first() && md <= last();
    }

    [[nodiscard]] friend constexpr bool operator==(quarter const &lhs, quarter const &rhs) noexcept { return lhs.q == rhs.q; }
    [[nodiscard]] friend constexpr bool operator!=(quarter const &lhs, quarter const &rhs) noexcept { return lhs.q != rhs.q; }
    [[nodiscard]] friend constexpr bool operator<(quarter const &lhs, quarter const &rhs) noexcept { return lhs.q < rhs.q; }
    [[nodiscard]] friend constexpr bool operator>(quarter const &lhs, quarter const &rhs) noexcept { return lhs.q > rhs.q; }
    [[nodiscard]] friend constexpr bool operator<=(quarter const &lhs, quarter const &rhs) noexcept { return lhs.q <= rhs.q; }
    [[nodiscard]] friend constexpr bool operator>=(quarter const &lhs, quarter const &rhs) noexcept { return lhs.q >= rhs.q; }

    [[nodiscard]] friend std::string to_string(quarter const &rhs) noexcept {
        return fmt::format("{}", rhs.q);
    }

    friend std::ostream &operator<<(std::ostream &lhs, quarter const &rhs) noexcept {
        return lhs << to_string(rhs);
    }

};

class year_quarter {
    date::year y;
    quarter q;

public:
    explicit constexpr year_quarter(date::year_month const &ym) :
        y(ym.year()), q(ym.month()) {}

    explicit constexpr year_quarter(date::year_month_day const &ymd) :
        year_quarter(date::year_month{ymd.year(), ymd.month()}) {}

    constexpr year_quarter &operator++() noexcept {
        if (q.increment_carry()) {
            ++y;
        }
        return *this;
    }

    [[nodiscard]] constexpr date::year_month first_year_month() noexcept {
        return date::year_month{y, q.first_month()};
    }

    [[nodiscard]] constexpr date::year_month last_year_month() noexcept {
        return date::year_month{y, q.last_month()};
    }

    [[nodiscard]] constexpr date::year_month_day first() noexcept {
        ttlet md = q.first();
        return date::year_month_day{y, md.month(), md.day()};
    }

    [[nodiscard]] constexpr date::year_month_day last() noexcept {
        ttlet md = q.last();
        return date::year_month_day{y, md.month(), md.day()};
    }

    [[nodiscard]] bool contains(date::year_month_day const &ymd) const noexcept {
        return y == ymd.year() && q.contains(date::month_day(ymd.month(), ymd.day()));
    }

    [[nodiscard]] friend constexpr bool operator==(year_quarter const &lhs, year_quarter const &rhs) noexcept {
        return lhs.y == rhs.y && lhs.q == rhs.q;
    }

    [[nodiscard]] friend constexpr bool operator<(year_quarter const &lhs, year_quarter const &rhs) noexcept {
        if (lhs.y == rhs.y) {
            return lhs.q < rhs.q;
        } else {
            return lhs.y < rhs.y;
        }
    }

    [[nodiscard]] friend constexpr bool operator!=(year_quarter const &lhs, year_quarter const &rhs) noexcept { return !(lhs == rhs); }
    [[nodiscard]] friend constexpr bool operator>(year_quarter const &lhs, year_quarter const &rhs) noexcept { return rhs < lhs; }
    [[nodiscard]] friend constexpr bool operator<=(year_quarter const &lhs, year_quarter const &rhs) noexcept { return !(lhs > rhs); }
    [[nodiscard]] friend constexpr bool operator>=(year_quarter const &lhs, year_quarter const &rhs) noexcept { return !(lhs < rhs); }

    [[nodiscard]] friend std::string to_string(year_quarter const &rhs) noexcept {
        return fmt::format("{}Q{}", rhs.y, rhs.q);
    }

    friend std::ostream &operator<<(std::ostream &lhs, year_quarter const &rhs) noexcept {
        return lhs << to_string(rhs);
    }
};


}