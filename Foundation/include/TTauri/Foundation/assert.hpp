// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/os_detect.hpp"

namespace TTauri {

[[noreturn]] void assertIsFatal(const char *source_file, int source_line, const char *message);

void assertIsLogged(const char *source_file, int source_line, const char *message);

#define AI_SKIP 'S'
#define AI_LOG 'L'
#define AI_TERMINATE 'T'
#define AI_ASSUME 'A'

#if defined(NDEBUG)
#define OPTIONAL_ASSERT_IMPLEMENTATION AI_SKIP
#define REVIEW_ASSERT_IMPLEMENTATION AI_LOG
#define REQUIRED_ASSERT_IMPLEMENTATION AI_TERMINATE
#define AXIOM_ASSERT_IMPLEMENTATION AI_ASSUME
#else
#define OPTIONAL_ASSERT_IMPLEMENTATION AI_TERMINATE
#define REVIEW_ASSERT_IMPLEMENTATION AI_TERMINATE
#define REQUIRED_ASSERT_IMPLEMENTATION AI_TERMINATE
#define AXIOM_ASSERT_IMPLEMENTATION AI_TERMINATE
#endif

#if OPTIONAL_ASSERT_IMPLEMENTATION == AI_SKIP
#define optional_assert(x) static_assert(sizeof(x) == 1)
#elif OPTIONAL_ASSERT_IMPLEMENTATION == AI_LOG
#define optional_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assertIsLogged(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif OPTIONAL_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define optional_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assertIsFatal(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif OPTIONAL_ASSERT_IMPLEMENTATION == AI_ASSUME
#define optional_assert(x) ttauri_assume(sizeof(x) == 1)
#else
#error "Optional assert implementation not set."
#endif

#if REVIEW_ASSERT_IMPLEMENTATION == AI_SKIP
#define review_assert(x) static_assert(sizeof(x) == 1)
#elif REVIEW_ASSERT_IMPLEMENTATION == AI_LOG
#define review_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assertIsLogged(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif REVIEW_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define review_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assertIsFatal(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif REVIEW_ASSERT_IMPLEMENTATION == AI_ASSUME
#define review_assert(x) ttauri_assume(sizeof(x) == 1)
#else
#error "Review assert implementation not set."
#endif

#if REQUIRED_ASSERT_IMPLEMENTATION == AI_SKIP
#define required_assert(x) static_assert(sizeof(x) == 1)
#elif REQUIRED_ASSERT_IMPLEMENTATION == AI_LOG
#define required_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assertIsLogged(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif REQUIRED_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define required_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assertIsFatal(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif REQUIRED_ASSERT_IMPLEMENTATION == AI_ASSUME
#define required_assert(x) ttauri_assume(sizeof(x) == 1)
#else
#error "Foundation assert implementation not set."
#endif

#if AXIOM_ASSERT_IMPLEMENTATION == AI_SKIP
#define axiom_assert(x) static_assert(sizeof(x) == 1)
#elif AXIOM_ASSERT_IMPLEMENTATION == AI_LOG
#define axiom_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assertIsLogged(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif AXIOM_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define axiom_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assertIsFatal(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif AXIOM_ASSERT_IMPLEMENTATION == AI_ASSUME
#define axiom_assert(x) ttauri_assume(sizeof(x) == 1)
#else
#error "Axiom assert implementation not set."
#endif

#define hresult_assert(x) ([](HRESULT result) {\
        if (ttauri_unlikely(FAILED(result))) {\
            assertIsFatal(__FILE__, __LINE__, fmt::format("Call to '{}' failed with {:08x}", #x, result).data());\
        }\
        return result;\
    }(x))

#define no_default assertIsFatal(__FILE__, __LINE__, "No default")
#define not_implemented assertIsFatal(__FILE__, __LINE__, "No implemented")
#define ttauri_overflow assertIsFatal(__FILE__, __LINE__, "Overflow")

}