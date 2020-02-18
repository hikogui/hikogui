// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/po_parser.hpp"

namespace TTauri::Text {




[[nodiscard]] Messages po_parser(std::string_view text) noexcept
{

}

[[nodiscard]] Messages po_parser(URL const &url) noexcept
{
    let v = FileView(url);
    return po_parser(v.string_view());
}


}

