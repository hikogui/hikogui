// Copyright 2019 Pokitec
// All rights reserved.

#include "exceptions.hpp"
#include "URL.hpp"

namespace TTauri {

void io_error::prepare_what() const noexcept {
    error::prepare_what();
    if (let url = get<URL,"url"_tag>()) {
        _what += fmt::format(" (url={0})", to_string(*url));
    }
}

}