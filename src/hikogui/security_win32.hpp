

#include "security.hpp"
#include <windows.h>

namespace tt::inline v1 {


void secure_clear(void *ptr, size_t size) noexcept
{
    SecureZeroMemory(ptr, size);
}

}

