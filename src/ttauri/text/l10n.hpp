// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "translation.hpp"
#include <string>

namespace tt {

class l10n {
public:
    [[nodiscard]] l10n() noexcept : msgid() {}

    [[nodiscard]] l10n(std::u8string msgid) noexcept : msgid(std::move(msgid)) {}

    [[nodiscard]] operator std::u8string_view() const noexcept
    {
        return get_translation(msgid);
    }

    [[nodiscard]] friend bool operator==(l10n const &lhs, l10n const &rhs) noexcept
    {
        return lhs.msgid == rhs.msgid;
    }

    //[[nodiscard]] friend auto operator<=>(l10n const &lhs, l10n const &rhs) noexcept
    //{
    //    return lhs.msgid <=> rhs.msgid;
    //}

private:
    std::u8string msgid;
};

} // namespace tt
