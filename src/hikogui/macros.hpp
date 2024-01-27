// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <version>
#include <exception>

#define HI_OS_WINDOWS 'W'
#define HI_OS_ANDROID 'A'
#define HI_OS_LINUX 'L'
#define HI_OS_MACOS 'M'
#define HI_OS_IOS 'I'

// W don't use HI_GENERIC for the operating system, because too many things
// like mmap-file-io, vulkan, window, main-loop depend on this.
#if defined(_WIN32)
#define HI_OPERATING_SYSTEM HI_OS_WINDOWS
#elif defined(__ANDROID__)
#define HI_OPERATING_SYSTEM HI_OS_ANDROID
#elif defined(__linux__)
#define HI_OPERATING_SYSTEM HI_OS_LINUX
#elif defined(__APPLE__) and defined(__MACH__)
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR or TARGET_OS_IPHONE or TARGET_OS_MACCATALYST
#define HI_OPERATING_SYSTEM HI_OS_IOS
#else
#define HI_OPERATING_SYSTEM HI_OS_MACOS
#endif
#else
#error "Unknown operating system"
#endif

#define HI_CC_MSVC 'm'
#define HI_CC_GCC 'g'
#define HI_CC_CLANG 'c'
#define HI_CC_OTHER '-'

#if defined(HI_GENERIC)
#define HI_COMPILER HI_CC_OTHER
#elif defined(__clang__)
#define HI_COMPILER HI_CC_CLANG
#elif defined(_MSC_BUILD)
#define HI_COMPILER HI_CC_MSVC
#elif defined(__GNUC__)
#define HI_COMPILER HI_CC_GCC
#else
#define HI_COMPILER HI_CC_OTHER
#endif

#define HI_STL_MS 'm'
#define HI_STL_GNU 'g'
#define HI_STL_LLVM 'l'
#define HI_STL_OTHER '-'

#if defined(HI_GENERIC)
#define HI_STD_LIBRARY HI_STL_OTHER
#elif defined(__GLIBCXX__)
#define HI_STD_LIBRARY HI_STL_GNU
#elif defined(_LIBCPP_VERSION)
#define HI_STD_LIBRARY HI_STL_LLVM
#elif defined(_CPPLIB_VER)
#define HI_STD_LIBRARY HI_STL_MS
#else
#define HI_STD_LIBRARY HI_STL_OTHER
#endif

#define HI_CPU_X86 'i'
#define HI_CPU_X86_64 'I'
#define HI_CPU_ARM 'a'
#define HI_CPU_ARM64 'A'
#define HI_CPU_OTHER '-'

#if defined(HI_GENERIC)
#define HI_PROCESSOR HI_CPU_OTHER
#elif defined(__amd64__) or defined(__amd64) or defined(__x86_64__) or defined(__x86_64) or defined(_M_AMD64) or defined(_M_X64)
#define HI_PROCESSOR HI_CPU_X86_64
#elif defined(__aarch64__) or defined(_M_ARM64)
#define HI_PROCESSOR HI_CPU_ARM64
#elif defined(__i386__) or defined(_M_IX86)
#define HI_PROCESSOR HI_CPU_X86
#elif defined(__arm__) or defined(__arm) or defined(_ARM) or defined(_M_ARM)
#define HI_PROCESSOR HI_CPU_ARM
#else
#define HI_PROCESSOR HI_CPU_OTHER
#endif

// All the HI_HAS_* macros tell us if the compiler will generate code with these instructions.
// Therefor we can use intrinsics for these instructions without checking the cpu-id.
// Which instrinsics are available is handled by a different macro.

#if HI_PROCESSOR == HI_CPU_X86_64
#define HI_HAS_X86_64 1
#define HI_HAS_X86 1
#elif HI_PROCESSOR == HI_CPU_X86
#define HI_HAS_X86 1
#endif

// Detect microarchitecture level.
#if defined(HI_HAS_X86)
#if HI_COMPILER == HI_CC_MSVC
// MSVC /arch:SSE (x86)
#if _M_IX86_FP >= 1 || defined(HI_HAS_X86_64)
#define HI_HAS_SSE 1
#define HI_HAS_MMX 1

#define HI_HAS_SCE 1
#define HI_HAS_OSFXSR 1
#define HI_HAS_FXSR 1
#define HI_HAS_FPU 1
#define HI_HAS_CX8 1
#define HI_HAS_CMOV 1

// MSVC /arch:SSE2 (x86)
#if _M_IX86_FP >= 2 || defined(HI_HAS_X86_64)
#define HI_HAS_SSE2 1

// MSVC /arch:AVX
#if defined(__AVX__)
// x86-64-v2
#define HI_HAS_SSSE3 1
#define HI_HAS_SSE4_1 1
#define HI_HAS_SSE4_2 1
#define HI_HAS_SSE3 1
#define HI_HAS_POPCNT 1
#define HI_HAS_LAHF 1
#define HI_HAS_CX16 1

#define HI_HAS_AVX 1

// MSVC /arch:AVX2
#if defined(__AVX2__)
// x86-64-v3
#define HI_HAS_AVX 1
#define HI_HAS_AVX2 1
#define HI_HAS_BMI1 1
#define HI_HAS_BMI2 1
#define HI_HAS_F16C 1
#define HI_HAS_FMA 1
#define HI_HAS_LZCNT 1
#define HI_HAS_MOVBE 1
#define HI_HAS_OSXSAVE 1

// MSVC /arch:AVX512
#if defined(__AVX512F__) && defined(__AVX512BW__) && defined(__AVX512CD__) && defined(__AVX512DQ__) && defined(__AVX512VL__)
// x86-64-v4
#define HI_HAS_AVX512F 1
#define HI_HAS_AVX512BW 1
#define HI_HAS_AVX512CD 1
#define HI_HAS_AVX512DQ 1
#define HI_HAS_AVX512VL 1

#endif // defined(__AVX512__)
#endif // defined(__AVX2__)
#endif // defined(__AVX__)
#endif // _M_IX86_FP >= 2
#endif // _M_IX86_FP >= 1

#elif HI_COMPILER == HI_CC_CLANG || HI_COMPILER == HI_CC_GCC
#if defined(__MMX__)
#define HI_HAS_MMX 1
#endif
#if defined(__SSE__)
#define HI_HAS_SSE 1
#endif
#if defined(__SSE2__)
#define HI_HAS_SSE2 1
#endif
#if defined(__SSE3__)
#define HI_HAS_SSE3 1
#endif
#if defined(__SSSE3__)
#define HI_HAS_SSSE3 1
#endif
#if defined(__SSE4_1__)
#define HI_HAS_SSE4_1 1
#endif
#if defined(__SSE4_2__)
#define HI_HAS_SSE4_2 1
#endif
#if defined(__POPCNT__)
#define HI_HAS_POPCNT 1
#endif
#if defined(__LAHF_SAHF__)
#define HI_HAS_LAHF 1
#endif
#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_16)
#define HI_HAS_CX16 1
#endif
#if defined(__AVX__)
#define HI_HAS_AVX 1
#endif
#if defined(__AVX2__)
#define HI_HAS_AVX2 1
#endif
#if defined(__BMI__)
#define HI_HAS_BMI1 1
#endif
#if defined(__BMI2__)
#define HI_HAS_BMI2 1
#endif
#if defined(__F16C__)
#define HI_HAS_F16C 1
#endif
#if defined(__FMA__)
#define HI_HAS_FMA 1
#endif
#if defined(__LZCNT__)
#define HI_HAS_LZCNT 1
#endif
#if defined(__MOVBE__)
#define HI_HAS_MOVBE 1
#endif
#if defined(__XSAVE__)
#define HI_HAS_XSAVE 1
#endif
#if defined(__AVX512BW__)
#define HI_HAS_AVX512BW 1
#endif
#if defined(__AVX512CD__)
#define HI_HAS_AVX512CD 1
#endif
#if defined(__AVX512DQ__)
#define HI_HAS_AVX512DQ 1
#endif
#if defined(__AVX512F__)
#define HI_HAS_AVX512F 1
#endif
#if defined(__AVX512VL__)
#define HI_HAS_AVX512VL 1
#endif
#if defined(__SHA__)
#define HI_HAS_SHA 1
#endif
#if defined(__AES__)
#define HI_HAS_AES 1
#endif
#if defined(__PCLMUL__)
#define HI_HAS_PCLMUL 1
#endif
#if defined(__RDRND__)
#define HI_HAS_RDRND 1
#endif
#if defined(__RDSEED__)
#define HI_HAS_RDSEED 1
#endif

#else // HI_COMPILER == HI_CC_CLANG || HI_COMPILER == HI_CC_GCC
#error "Unknown compiler for x86"
#endif

#endif // defined(HI_HAS_X86)

// clang-format off
#if defined(HI_HAS_SSE2) && defined(HI_HAS_SSE) && defined(HI_HAS_SCE) && \
    defined(HI_HAS_OSFXSR) && defined(HI_HAS_MMX) && defined(HI_HAS_FXSR) && \
    defined(HI_HAS_FPU) && defined(HI_HAS_CX8) && defined(HI_HAS_CMOV)
#define HI_HAS_X86_64_V1 1
#endif

#if defined(HI_HAS_X86_64_V1) && defined(HI_HAS_SSSE3) && defined(HI_HAS_SSE4_1) && \
    defined(HI_HAS_SSE4_2) && defined(HI_HAS_SSE3) && defined(HI_HAS_POPCNT) && \
    defined(HI_HAS_LAHF) && defined(HI_HAS_CX16)
#define HI_HAS_X86_64_V2 1
#endif

#if defined(HI_HAS_X86_64_V2) && defined(HI_HAS_AVX) && defined(HI_HAS_AVX2) && \
    defined(HI_HAS_BMI1) && defined(HI_HAS_BMI2) && defined(HI_HAS_F16C) && \
    defined(HI_HAS_FMA) && defined(HI_HAS_LZCNT) && defined(HI_HAS_MOVBE) && \
    defined(HI_HAS_OSXSAVE)
#define HI_HAS_X86_64_V3 1
#endif

#if defined(HI_HAS_X86_64_V3) && defined(HI_HAS_AVX512F) && defined(HI_HAS_AVX512BW) && \
    defined(HI_HAS_AVX512CD) && defined(HI_HAS_AVX512DQ) && defined(HI_HAS_AVX512VL)
#define HI_HAS_X86_64_V4 1
#endif
// clang-format on

inline void weak_terminate() noexcept
{
    std::terminate();
}

/** Assert Break.
 *
 * This will cause a trap on the processor in case of an assert.
 *
 * With the following goal:
 *  - Optionally launch a just-in-time debugger.
 *  - With debugger: Allow the debugger to continue beyond the trap.
 *  - Without debugger: Terminate the application with an error.
 */
#if HI_COMPILER == HI_CC_CLANG
#define hi_assert_break() __builtin_trap()
#elif HI_COMPILER == HI_CC_MSVC
#define hi_assert_break() __int2c()
#elif HI_COMPILER == HI_CC_GCC
#define hi_assert_break() __builtin_trap()
#else
#define hi_assert_break() weak_terminate()
#endif

/** Debug Break.
 *
 * This will cause a trap on the processor for a break point in the debugger.
 *
 * With the following goal:
 *  - Optionally launch a just-in-time debugger.
 *  - With debugger: Allow the debugger to continue beyond the trap.
 *  - Without debugger: Continue beyond the trap.
 */
#if HI_COMPILER == HI_CC_CLANG
#define hi_debug_break() __builtin_debugtrap()
#elif HI_COMPILER == HI_CC_MSVC
#define hi_debug_break() __debugbreak()
#elif HI_COMPILER == HI_CC_GCC
#define hi_debug_break() __builtin_trap()
#else
#define hi_debug_break() []{std::terminate();}()
#endif

/** Override architecture and instruction-sets for a function.
 *
 * The gcc and clang compiler require that a function is decorated with
 * the architecture and instruction-sets for the intrinsics used in this
 * function.
 *
 * This attributes is ignored on MSVC, which can use intrinsics outside
 * of the architecture passed on the command line.
 *
 * @param ... A string with comma separated architecture and instruction sets to use.
 */
#if HI_COMPILER == HI_CC_CLANG
#define hi_target(...) [[gnu::target(__VA_ARGS__)]]
#elif HI_COMPILER == HI_CC_MSVC
#define hi_target(...)
#elif HI_COMPILER == HI_CC_GCC
#define hi_target(...) [[gnu::target(__VA_ARGS__)]]
#else
#define hi_target(...)
#endif

/** Assume an expression to be true.
 *
 * Equivilant to C++23 [[assume(expression)]] attribute.
 */
#if HI_COMPILER == HI_CC_CLANG
#define hi_assume(...) __builtin_assume(not not(__VA_ARGS__))
#elif HI_COMPILER == HI_CC_MSVC
#define hi_assume(...) __assume(__VA_ARGS__)
#elif HI_COMPILER == HI_CC_GCC
#define hi_assume(...) \
    do { \
        if (not(__VA_ARGS__)) { \
            std::unreachable(); \
        } \
    } while (false)
#else
#define hi_assume(...) static_assert(sizeof(__VA_ARGS__) == 1)
#endif

/** Force inline (optimization)
 *
 * Force the instructions of a function to be inlined at the
 * call site. This is an optimization tool.
 *
 * Neither `hi_force_inline` and `hi_no_inline` imply the C++ keyword `inline`.
 *
 * Although `hi_force_inline` seems like an easy way to force a certain type
 * of optimization, I recommend `hi_no_inline` instead which often has a lot
 * more impact on optimiation by reducing code complexity by removing
 * code that is not important.
 */
#if defined(NDEBUG)
#if HI_COMPILER == HI_CC_CLANG
#define hi_force_inline __attribute__((always_inline))
#elif HI_COMPILER == HI_CC_MSVC
#define hi_force_inline __forceinline
#elif HI_COMPILER == HI_CC_GCC
#define hi_force_inline __attribute__((always_inline))
#else
#define hi_force_inline __attribute__((always_inline))
#endif
#else
#define hi_force_inline
#endif

/** Function decorator to make sure that the function will not be inlined.
*/
#if HI_COMPILER == HI_CC_CLANG
#define hi_no_inline __attribute__((noinline))
#elif HI_COMPILER == HI_CC_MSVC
#define hi_no_inline __declspec(noinline)
#elif HI_COMPILER == HI_CC_GCC
#define hi_no_inline __attribute__((noinline))
#else
#define hi_no_inline
#endif

/** Pointer decorator to tell the optimizer that the pointer does not alias.
 */
#if HI_COMPILER == HI_CC_CLANG
#define hi_restrict __restrict__
#elif HI_COMPILER == HI_CC_MSVC
#define hi_restrict __restrict
#elif HI_COMPILER == HI_CC_GCC
#define hi_restrict __restrict__
#else
#define hi_restrict
#endif

/** Function decorator to turn of address sanitizer inside the function.
 */
#if HI_COMPILER == HI_CC_CLANG
#define hi_no_sanitize_address
#elif HI_COMPILER == HI_CC_MSVC
#define hi_no_sanitize_address __declspec(no_sanitize_address)
#elif HI_COMPILER == HI_CC_GCC
#define hi_no_sanitize_address
#else
#define hi_no_sanitize_address
#endif

// Even when HI_GENERIC is set, we want the warning for the specific compilers
// to be properly suppressed.
#if defined(__clang__)
#define hi_warning_push() _Pragma("warning(push)")
#define hi_warning_pop() _Pragma("warning(push)")
#define hi_warning_ignore_msvc(...)
#define hi_warning_ignore_clang(...) _Pragma(hi_stringify(clang diagnostic ignored __VA_ARGS__))
#define hi_warning_ignore_gcc(...)
#elif defined(_MSC_BUILD)
#define hi_warning_push() _Pragma("warning( push )")
#define hi_warning_pop() _Pragma("warning( pop )")
#define hi_msvc_pragma(...) _Pragma(__VA_ARGS__)
#define hi_warning_ignore_msvc(...) _Pragma(hi_stringify(warning(disable : __VA_ARGS__)))
#define hi_warning_ignore_clang(...)
#define hi_warning_ignore_gcc(...)
#elif defined(__GNUC__)
#define hi_warning_push() _Pragma("warning(push)")
#define hi_warning_pop() _Pragma("warning(pop)")
#define hi_warning_ignore_msvc(...)
#define hi_warning_ignore_clang(...)
#define hi_warning_ignore_gcc(...) _Pragma(hi_stringify(clang diagnostic ignored __VA_ARGS__))
#else
#define hi_warning_push()
#define hi_warning_pop()
#define hi_warning_ignore_msvc(...)
#define hi_warning_ignore_clang(...)
#define hi_warning_ignore_gcc(...)
#endif


/** After module translation "hi_export_module(x)" is replaced with "export module x".
 */
#ifndef hi_export_module
#define hi_export_module(x)
#endif

/** After module translation "hi_export " is replace with "export ".
 */
#ifndef hi_export
#define hi_export
#endif

/** After module translation "hi_inline " is replaced with "".
 */
#ifndef hi_inline
#define hi_inline inline
#endif

/** Invariant should be the default for variables.
 *
 * C++ does have an invariant but it requires you to enter the 'const' keyword which
 * is easy to forget. Using a single keyword 'hilet' for an invariant makes it easier to notice
 * when you have defined a variant.
 */
#ifndef hilet
#define hilet auto const
#endif

/** Forward a value, based on the decltype of the value.
 */
#ifndef hi_forward
#define hi_forward(x) std::forward<decltype(x)>(x)
#endif

/** Return the result of an expression if the expression is valid.
 *
 * This macro uses a `requires {}` expression to determine if the expression is valid.
 *
 * @param expression The expression to evaluate if it is valid.
 */
#define hi_return_if_valid(expression) \
    do { \
        if constexpr (requires { expression; }) { \
            return expression; \
        } \
    } while (false)

// One clang-format off is not enough to stop clang-format to format.
// clang-format off
#define hi_num_va_args_impl( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define hi_num_va_args_(...) hi_num_va_args_impl(__VA_ARGS__)
// clang-format on        

/** Get the number of arguments.
 * 
 * @param ... A set of arguments
 * @return The number of arguments pass to this macro.
 */
// clang-format off
#define hi_num_va_args(...) hi_num_va_args_(__VA_ARGS__ __VA_OPT__(,) \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0)
// clang-format on

#define hi_parans ()

#define hi_expand1(...) __VA_ARGS__
#define hi_expand2(...) hi_expand1(hi_expand1(hi_expand1(hi_expand1(__VA_ARGS__))))
#define hi_expand3(...) hi_expand2(hi_expand2(hi_expand2(hi_expand2(__VA_ARGS__))))
#define hi_expand4(...) hi_expand3(hi_expand3(hi_expand3(hi_expand3(__VA_ARGS__))))

/** Re-evaluate text up to 256 times by the pre-processor.
 *
 * @param ... The text which needs to be re-evaluated by the pre-processor.
 */
#define hi_expand(...) hi_expand4(hi_expand4(hi_expand4(hi_expand4(__VA_ARGS__))))

#define hi_for_each_again() hi_for_each_helper
#define hi_for_each_helper(macro, first_arg, ...) macro(first_arg) __VA_OPT__(hi_for_each_again hi_parans(macro, __VA_ARGS__))

/** Evaluate a macro for each argument.
 *
 * @param macro A macro that accepts a single argument.
 * @param ... A set of arguments to be passed one-by-one to @a macro.
 */
#define hi_for_each(macro, ...) __VA_OPT__(hi_expand(hi_for_each_helper(macro, __VA_ARGS__)))

#define hi_stringify_(x) #x
#define hi_stringify(x) hi_stringify_(x)

#define hi_cat_(a, b) a##b
#define hi_cat(a, b) hi_cat_(a, b)

#define hi_return_on_self_assignment(other) \
    if (&(other) == this) [[unlikely]] \
        return *this;

/** Get an overloaded macro for 1 or 2 arguments.
 *
 * This macro allows dispatching to other macros based on the number of arguments.
 * ```
 * #define foo1(a) bar(a)
 * #define foo2(a, b) bar(a, b)
 * #define foo(...) hi_get_overloaded_macro2(__VA_ARGS__, foo2, foo1)(__VA_ARGS__)
 * ```
 */
#define hi_get_overloaded_macro2(_1, _2, name, ...) name

#define ssizeof(x) (static_cast<ssize_t>(sizeof(x)))

/** Debug-break and abort the application.
 *
 * This function will break the application in the debugger.
 * Potentially it will start the Just-In-Time debugger if one is configured.
 *
 * If no debugger is present than the application will be aborted with `std::terminate()`.
 * If the debugger is attached, it is allowed to continue.
 *
 * It is possible for c-style `assert()` to display the error message of an
 * earlier `hi_assert_abort()` (together with the error message from `assert()`),
 * if the earlier `hi_assert_abort()` was continued by a debugger.
 *
 * @param ... The reason why the abort is done.
 */
#define hi_assert_abort(...) \
    do { \
        ::hi::set_debug_message(__FILE__ ":" hi_stringify(__LINE__) ":" __VA_ARGS__); \
        hi_assert_break(); \
    } while (false)

/** Check if the expression is valid, or throw a parse_error.
 *
 * This function is used to check if an expression is correct during the
 * parsing of data.
 *
 * @param expression The expression to check.
 * @param message The message to set in the parse_error.
 * @param ... Optional format parameters for the message.
 * @throws parse_error When the expression yields false.
 */
#define hi_check(expression, message, ...) \
    do { \
        if (not(expression)) { \
            if constexpr (__VA_OPT__(not ) false) { \
                throw parse_error(std::format(message __VA_OPT__(, ) __VA_ARGS__)); \
            } else { \
                throw parse_error(message); \
            } \
        } \
    } while (false)

/** Assert if a value is within bounds, or throw a parse_error.
 *
 * This function is used to check if a value is within bounds
 * during parsing of data.
 *
 * Lower-bound is inclusive and Upper-bound is exclusive.
 *
 * @param x The value to check if it is within bounds.
 * @param ... One upper-bound; or a lower-bound and upper-bound.
 * @throws parse_error When the expression yields false.
 */
#define hi_check_bounds(x, ...) \
    do { \
        if (not ::hi::bound_check(x, __VA_ARGS__)) { \
            throw parse_error("assert bounds: " hi_stringify(x) " between " hi_for_each(hi_stringify, (__VA_ARGS__))); \
        } \
    } while (false)

/** Get a subspan, or throw a parse_error.
 *
 * This function is used to get a subspan of data with bounds
 * checks during parsing of data.
 *
 * @param span The span to take the subspan of.
 * @param offset The offset within the span to start the subspan.
 * @param ... Optional count for the number of elements in the subspan.
 *            When not specified the subspan is up to the end of the span.
 * @return A subspan.
 * @throws parse_error When the subspan does not fit the given span.
 */
#define hi_check_subspan(span, offset, ...) \
    [&](auto _hi_check_subspan_span, size_t _hi_check_subspan_offset, auto... _hi_check_subspan_count) { \
        if constexpr (sizeof...(_hi_check_subspan_count) == 0) { \
            if (_hi_check_subspan_offset < _hi_check_subspan_span.size()) { \
                return _hi_check_subspan_span.subspan(_hi_check_subspan_offset); \
            } \
        } else if constexpr (sizeof...(_hi_check_subspan_count) == 1) { \
            if (_hi_check_subspan_offset + wide_cast<size_t>(_hi_check_subspan_count...) <= _hi_check_subspan_span.size()) { \
                return _hi_check_subspan_span.subspan(_hi_check_subspan_offset, _hi_check_subspan_count...); \
            } \
        } \
        throw parse_error( \
            "assert bounds on: " hi_stringify(span) ".subspan(" hi_stringify(offset __VA_OPT__(", ") __VA_ARGS__) ")"); \
    }(span, offset __VA_OPT__(, ) __VA_ARGS__)

/** Get an element from a span, or throw a parse_error.
 *
 * This function is used to get an element of span with bounds
 * checks during parsing of data.
 *
 * @param span The span to take the subspan of.
 * @param index The index to the element in the span.
 * @return A reference to the element.
 * @throws parse_error When the index is not contained in the given span.
 */
#define hi_check_at(span, index) \
    [&](auto _hi_check_subspan_span, size_t _hi_check_subspan_index) { \
        if (_hi_check_subspan_index < _hi_check_subspan_span.size()) { \
            return _hi_check_subspan_span[_hi_check_subspan_index]; \
        } else { \
            throw parse_error("assert bounds on: " hi_stringify(span) "[" hi_stringify(index) "]"); \
        } \
    }(span, index)

#define hi_hresult_check(expression) \
    ([](HRESULT result) { \
        if (FAILED(result)) { \
            throw ::hi::io_error(std::format("Call to '{}' failed with {:08x}", #expression, result)); \
        } \
        return result; \
    }(expression))

/** Assert if expression is true.
 * Independent of built type this macro will always check and abort on fail.
 *
 * @param expression The expression to test.
 * @param ... A string-literal explaining the reason why this assert exists.
 */
#define hi_assert(expression, ...) \
    do { \
        if (not(expression)) { \
            hi_assert_abort("assert: " __VA_ARGS__ " not (" hi_stringify(expression) ")"); \
        } \
    } while (false)

/** Assert if an expression is true.
 * If the expression is false then return from the function.
 *
 * @param x The expression to test
 * @param y The value to return from the current function.
 */
#define hi_assert_or_return(x, y) \
    if (!(x)) { \
        [[unlikely]] return y; \
    }

/** Assert if a value is within bounds.
 * Independent of built type this macro will always check and abort on fail.
 *
 * Lower-bound is inclusive and Upper-bound is exclusive.
 *
 * @param x The value to check if it is within bounds.
 * @param ... One upper-bound; or a lower-bound and upper-bound.
 */
#define hi_assert_bounds(x, ...) \
    do { \
        if (not ::hi::bound_check(x, __VA_ARGS__)) { \
            hi_assert_abort("assert bounds: " hi_stringify(x) " between " hi_for_each(hi_stringify, (__VA_ARGS__))); \
        } \
    } while (false)

/** Assert if an expression is not nullptr.
 * If the expression is not a nullptr then return from the function.
 *
 * @param x The expression to test
 * @param ... A string-literal as the reason why the not-null check exists.
 */
#define hi_assert_not_null(x, ...) \
    do { \
        if (x == nullptr) { \
            hi_assert_abort("assert not-null: " __VA_ARGS__ " (" hi_stringify(x) ")"); \
        } \
    } while (false)

/** Specify an axiom; an expression that is true.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 *
 * @param expression The expression that is true.
 * @param ... A string-literal as the reason why the axiom exists.
 */
#ifndef NDEBUG
#define hi_axiom(expression, ...) hi_assert(expression __VA_OPT__(, ) __VA_ARGS__)
#else
#define hi_axiom(expression, ...) hi_assume(expression)
#endif

/** Specify an axiom that the value is within bounds.
 * An axiom is checked in debug mode, and is used as an optimization
 * in release mode.
 *
 * Lower-bound is inclusive and Upper-bound is exclusive.
 *
 * @param x The value to check if it is within bounds.
 * @param ... One upper-bound; or a lower-bound and upper-bound.
 */
#ifndef NDEBUG
#define hi_axiom_bounds(x, ...) hi_assert_bounds(x, __VA_ARGS__)
#else
#define hi_axiom_bounds(x, ...) hi_assume(not ::hi::bound_check(x, __VA_ARGS__))
#endif

/** Assert if an expression is not nullptr.
 * If the expression is not a nullptr then return from the function.
 *
 * @param expression The expression to test
 * @param ... A string-literal as the reason why the not-null check exists.
 */
#ifndef NDEBUG
#define hi_axiom_not_null(expression, ...) hi_assert_not_null(expression __VA_OPT__(, ) __VA_ARGS__)
#else
#define hi_axiom_not_null(expression, ...) hi_assume(expression != nullptr)
#endif

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable else statements or switch-default labels,
 *
 * @param ... A string-literal as the reason why the no-default exists.
 */
#ifndef NDEBUG
#define hi_no_default(...) \
    [[unlikely]] hi_assert_abort("Reached no-default:" __VA_ARGS__); \
    std::terminate()
#else
#define hi_no_default(...) std::unreachable()
#endif

/** This part of the code should not be reachable, unless a programming bug.
 * This function should be used in unreachable constexpr else statements.
 *
 * @param ... A string-literal as the reason why the no-default exists.
 */
#define hi_static_no_default(...) \
    []<bool Flag = false>() \
    { \
        static_assert(Flag, "No default: " __VA_ARGS__); \
    } \
    ()

/** This part of the code has not been implemented yet.
 * This aborts the program.
 *
 * @param ... A string-literal as the reason why this it not implemented.
 */
#define hi_not_implemented(...) \
    [[unlikely]] hi_assert_abort("Not implemented: " __VA_ARGS__); \
    std::terminate()

/** This part of the code has not been implemented yet.
 * This function should be used in unreachable constexpr else statements.
 *
 * @param ... A string-literal as the reason why this it not implemented.
 */
#define hi_static_not_implemented(...) hi_static_no_default("Not implemented: " __VA_ARGS__)

/** Format and output text to the console.
 * This will output the text to the console's std::cout.
 * During debugging the console will be the debugger's output panel/window.
 *
 * @param text The text to display on the console.
 */
#define hi_print(fmt, ...) console_output(std::format(fmt __VA_OPT__(, ) __VA_ARGS__))

#define hi_format_argument_check(arg) \
    static_assert( \
        ::std::is_default_constructible_v<std::formatter<std::decay_t<decltype(arg)>>>, \
        "std::format, argument '" #arg "' does not have a specialized std::formatter<>.");

/** A macro to check if the format string and the arguments are valid for std::format.
 *
 * This macro checks if the usage of braces '{' and '}' are correctly used in the format-string.
 * Then it will check if the number of arguments match the number of arguments in the format-string.
 * Lastly it will check if the type for each argument has a valid `std::formatter<>` specialization.
 *
 * This is done in a macro instead of a function, so that the static_asserts will point to the line
 * where the format-string and arguments where defined.
 *
 * @param fmt The `std::format` format-string.
 * @param ... The arguments to be formatted by `std::format`.
 */
#define hi_format_check(fmt, ...) \
    static_assert(::hi::format_count(fmt) != -1, "std::format, Unexpected '{' inside argument-format."); \
    static_assert(::hi::format_count(fmt) != -2, "std::format, Unexpected '}' without corresponding '{'."); \
    static_assert(::hi::format_count(fmt) != -3, "std::format, Missing '}' at end of format string."); \
    static_assert( \
        ::hi::format_count(fmt) == hi_num_va_args(__VA_ARGS__), "std::format, invalid number of arguments for format string."); \
    hi_for_each(hi_format_argument_check, __VA_ARGS__)

#define hi_log(level, fmt, ...) \
    hi_format_check(fmt __VA_OPT__(, ) __VA_ARGS__); \
    ::hi::log_global.add<level, __FILE__, __LINE__, fmt>(__VA_ARGS__)

#define hi_log_debug(fmt, ...) hi_log(::hi::global_state_type::log_debug, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_info(fmt, ...) hi_log(::hi::global_state_type::log_info, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_statistics(fmt, ...) hi_log(::hi::global_state_type::log_statistics, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_trace(fmt, ...) hi_log(::hi::global_state_type::log_trace, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_audit(fmt, ...) hi_log(::hi::global_state_type::log_audit, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_warning(fmt, ...) hi_log(::hi::global_state_type::log_warning, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_error(fmt, ...) hi_log(::hi::global_state_type::log_error, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_fatal(fmt, ...) \
    hi_log(::hi::global_state_type::log_fatal, fmt __VA_OPT__(, ) __VA_ARGS__); \
    hi_assert_abort(); \
    std::terminate()

#define hi_log_info_once(name, fmt, ...) \
    do { \
        if (++::hi::global_counter<name> == 1) { \
            hi_log(::hi::global_state_type::log_info, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } \
    } while (false)

#define hi_log_error_once(name, fmt, ...) \
    do { \
        if (++::hi::global_counter<name> == 1) { \
            hi_log(::hi::global_state_type::log_error, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } \
    } while (false)

