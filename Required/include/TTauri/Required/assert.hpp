// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/os_detect.hpp"
#include <atomic>

namespace TTauri {

void stop_debugger();

/*! Add a message to the log about the failed assert.
* Optionally let the user specify what to do from a dialogue.
* * Abort -> std::terminate()
* * Retry -> Continue, when assert triggers again; the dialogue pops up again.
* * Ignore -> Continue, when assert triggers again; the assert is ignored.
*
* \param count The number of times this assert was triggered, starting at zero.
* \param source_file location where the assert is in the source code.
* \param source_line location where the assert is in the source code.
* \param terminate End the application.
*/
void assert_logging(uint64_t count, char const *source_file, int source_line, char const *expression);

[[noreturn]] void assert_terminating(char const *source_file, int source_line, char const *expression);


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
            static std::atomic<uint64_t> count;\
            assert_logging(count.fetch_add(1, std::memory_order_relaxed), __FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif OPTIONAL_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define optional_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assert_terminating(__FILE__, __LINE__, #x);\
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
            static std::atomic<uint64_t> count;\
            assert_logging(count.fetch_add(1, std::memory_order_relaxed), __FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif REVIEW_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define review_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assert_terminating(__FILE__, __LINE__, #x);\
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
            static std::atomic<uint64_t> count;\
            assert_logging(count.fetch_add(1, std::memory_order_relaxed), __FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif REQUIRED_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define required_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assert_terminating(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif REQUIRED_ASSERT_IMPLEMENTATION == AI_ASSUME
#define required_assert(x) ttauri_assume(sizeof(x) == 1)
#else
#error "Required assert implementation not set."
#endif

#if AXIOM_ASSERT_IMPLEMENTATION == AI_SKIP
#define axiom_assert(x) static_assert(sizeof(x) == 1)
#elif AXIOM_ASSERT_IMPLEMENTATION == AI_LOG
#define axiom_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            static std::atomic<uint64_t> count;\
            assert_logging(count.fetch_add(1, std::memory_order_relaxed), __FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif AXIOM_ASSERT_IMPLEMENTATION == AI_TERMINATE
#define axiom_assert(x)\
    do {\
        if (ttauri_unlikely(!(x))) {\
            assert_terminating(__FILE__, __LINE__, #x);\
        }\
    } while (false)
#elif AXIOM_ASSERT_IMPLEMENTATION == AI_ASSUME
#define axiom_assert(x) ttauri_assume(sizeof(x) == 1)
#else
#error "Axiom assert implementation not set."
#endif

#define hresult_assert(x) ([](HRESULT result) {\
        if (ttauri_unlikely(FAILED(result))) {\
            let message = fmt::format("Call to '{}' failed with {:08x}", #x, result);\
            assert_terminating(__FILE__, __LINE__, message.data());\
        }\
        return result;\
    }(x))


#define no_default assert_terminating(__FILE__, __LINE__, "No default")
#define not_implemented assert_terminating(__FILE__, __LINE__, "No implemented")
#define ttauri_overflow assert_terminating(__FILE__, __LINE__, "Overflow")

}