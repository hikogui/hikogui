// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "loop.hpp"

namespace hi::inline v1 {

[[nodiscard]] static loop &loop::main() noexcept
{
    if (auto *ptr = _main.get()) {
        return *ptr;
    } else {
        _main = std::make_unique<loop>();
        _main._is_main = true;
        return *_main;
    }
}

}

