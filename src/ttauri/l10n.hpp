// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

namespace tt {

/** A localizable string.
 * Used by gettext to extract all msgids from the program into the .pot file.
 */
class l10n {
public:
    l10n() noexcept : msgid() {}
    l10n(l10n const &) noexcept = default;
    l10n(l10n &&) noexcept = default;
    l10n &operator=(l10n const &) noexcept = default;
    l10n &operator=(l10n &&) noexcept = default;

    l10n(std::u8string_view msgid) noexcept : msgid(msgid) {}
    l10n(std::string_view msgid) noexcept
    {
        this->msgid = std::u8string(reinterpret_cast<char8_t const *>(msgid.data()), msgid.size());
    }

private:
    std::u8string msgid;

    friend class label;
};

}
