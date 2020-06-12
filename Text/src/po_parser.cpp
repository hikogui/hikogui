// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/po_parser.hpp"

namespace tt {




[[nodiscard]] Messages po_parser(std::string_view text) noexcept
{

}

[[nodiscard]] Messages po_parser(URL const &url) noexcept
{
    let v = ResourceView::loadView(url);
    return po_parser(v->string_view());
}


}

