// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "language_tag.hpp"
#include "language.hpp"
#include "translation.hpp"
#include "po_parser.hpp"
#include <fmt/format.h>

namespace tt {

language::language(language_tag tag) noexcept :
    tag(std::move(tag)), plurality_func()
{
    // XXX fmt::format is unable to find language_tag::operator<<
    auto po_url = URL(fmt::format("resource:locale/{}.po", to_string(this->tag)));

    tt_log_info("Loading language {} catalog {}", to_string(this->tag), po_url);

    try {
        add_translation(parse_po(po_url), *this);

    } catch (std::exception const &e) {
        tt_log_warning("Could not load language catalog {}: \"{}\"", this->tag, e.what());
    }
}

}