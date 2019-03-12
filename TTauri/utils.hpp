#pragma once

#include <cstdint>

namespace TTauri {

inline size_t align(size_t offset, size_t alignment) {
    return ((offset + alignment - 1) / alignment) * alignment;
}

}