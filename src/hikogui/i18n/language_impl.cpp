// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "language.hpp"
#include "translation.hpp"
#include "po_parser.hpp"
#include "../i18n/language_tag.hpp"
#include "../file/URL.hpp"
#include "../log.hpp"
#include <format>

namespace hi::inline v1 {

language::language(language_tag tag) noexcept : tag(std::move(tag)), plurality_func()
{
    // XXX std::format is unable to find language_tag::operator<<
    auto po_url = URL(std::format("resource:locale/{}.po", this->tag));

    hi_log_info("Loading language {} catalog {}", this->tag, po_url);

    try {
        add_translation(parse_po(po_url), *this);

    } catch (std::exception const &e) {
        hi_log_warning("Could not load language catalog {}: \"{}\"", this->tag, e.what());
    }
}

} // namespace hi::inline v1
