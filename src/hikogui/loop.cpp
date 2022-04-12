
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

