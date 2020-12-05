
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

    LOG_INFO("Loading language {} catalogue {}", to_string(this->tag), po_url);

    try {
        add_translation(parse_po(po_url), *this);

    } catch (std::exception const &e) {
        LOG_WARNING("Could not load language catalogue: {}", tt::to_string(e));
    }
}

}