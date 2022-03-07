// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "iso639.hpp"
#include "iso3166.hpp"
#include "iso15924.hpp"

namespace tt::inline v1 {

class language_tag {
public:

    [[nodiscard]] constexpr iso639 language() const noexcept
    {
        return _language;
    }

    [[nodiscard]] iso15924 script() const noexcept
    {
        if (_script) {
            return _script;
        } else {
            return _language.default_script();
        }
    }

private:
    iso639 _language;
    iso15924 _script;
    iso3166 _region;
};

}
