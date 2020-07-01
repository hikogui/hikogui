
#include "language.hpp"
#include "translation.hpp"
#include "po_parser.hpp"

namespace tt {

language::language(std::string tag) noexcept :
    tag(std::move(tag)), plurality_func()
{
    auto po_url = URL(fmt::format("resource:locale/{}.po", this->tag));

    LOG_INFO("Loading language {} catalogue {}", this->tag, po_url);

    try {
        add_translation(parse_po(po_url), *this);
    } catch (error &e) {
        LOG_WARNING("Could not load language catalogue: {}", to_string(e));
    }
}

}