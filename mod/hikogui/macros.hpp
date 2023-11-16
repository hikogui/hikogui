// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <version>

#ifndef HI_ASSERT_HPP
#define HI_ASSERT_HPP

#define HI_OS_WINDOWS 'W'
#define HI_OS_MACOS 'A'
#define HI_OS_MOBILE 'M'
#define HI_OS_OTHER 'O'

#if defined(_WIN32)
#define HI_OPERATING_SYSTEM HI_OS_WINDOWS
#elif defined(TARGET_OS_MAC) and not defined(TARGET_OS_IPHONE)
#define HI_OPERATING_SYSTEM HI_OS_MACOS
#elif defined(TARGET_OS_IPHONE) or defined(__ANDROID__)
#define HI_OPERATING_SYSTEM HI_OS_MOBILE
#else
#define HI_OPERATING_SYSTEM HI_OS_OTHER
#endif

#define HI_CC_MSVC 'm'
#define HI_CC_GCC 'g'
#define HI_CC_CLANG 'c'

#if defined(__clang__)
#define HI_COMPILER HI_CC_CLANG
#elif defined(_MSC_BUILD)
#define HI_COMPILER HI_CC_MSVC
#elif defined(__GNUC__)
#define HI_COMPILER HI_CC_GCC
#else
#error "Could not detect the compiler."
#endif

#define HI_STL_MS 'm'
#define HI_STL_GNU 'g'
#define HI_STL_LLVM 'l'
#define HI_STL_UNKNOWN '-'

#if defined(__GLIBCXX__)
#define HI_STD_LIBRARY HI_STL_GNU
#elif defined(_LIBCPP_VERSION)
#define HI_STD_LIBRARY HI_STL_LLVM
#elif defined(_CPPLIB_VER)
#define HI_STD_LIBRARY HI_STL_MS
#else
#define HI_STD_LIBRARY HI_STL_UNKNOWN
#endif

#define HI_CPU_X86 'i'
#define HI_CPU_X64 'I'
#define HI_CPU_ARM 'a'
#define HI_CPU_ARM64 'A'
#define HI_CPU_UNKNOWN '-'

#if defined(__amd64__) or defined(__amd64) or defined(__x86_64__) or defined(__x86_64) or defined(_M_AMD64) or defined(_M_X64)
#define HI_PROCESSOR HI_CPU_X64
#elif defined(__aarch64__) or defined(_M_ARM64)
#define HI_PROCESSOR HI_CPU_ARM64
#elif defined(__i386__) or defined(_M_IX86)
#define HI_PROCESSOR HI_CPU_X86
#elif defined(__arm__) or defined(__arm) or defined(_ARM) or defined(_M_ARM)
#define HI_PROCESSOR HI_CPU_ARM
#else
#define HI_PROCESSOR HI_CPU_UNKNOWN
#endif

#if defined(__AVX512BW__) and defined(__AVX512CD__) and defined(__AVX512DQ__) and defined(__AVX512F__) and defined(__AVX512VL__)
#define HI_X86_64_LEVEL 4
#elif defined(__AVX2__)
#define HI_X86_64_LEVEL 3
#elif defined(__SSE4_2__) and defined(__SSSE3__)
#define HI_X86_64_LEVEL 2
#elif HI_PROCESSOR == HI_CPU_X64
#define HI_X86_64_LEVEL 1
#endif

#if defined(HI_X86_64_MAX_LEVEL) and defined(HI_X86_64_LEVEL) and HI_X86_64_MAX_LEVEL < HI_X86_64_LEVEL
#undef HI_X86_64_LEVEL
#define HI_X86_64_LEVEL HI_X86_64_MAX_LEVEL
#endif

#if defined(HI_X86_64_LEVEL) and HI_X86_64_LEVEL >= 4
#define HI_HAS_AVX512F 1
#define HI_HAS_AVX512BW 1
#define HI_HAS_AVX512CD 1
#define HI_HAS_AVX512DQ 1
#define HI_HAS_AVX512VL 1
#endif

#if defined(HI_X86_64_LEVEL) and HI_X86_64_LEVEL >= 3
#define HI_HAS_AVX 1
#define HI_HAS_AVX2 1
#define HI_HAS_BMI1 1
#define HI_HAS_BMI2 1
#define HI_HAS_F16C 1
#define HI_HAS_FMA 1
#define HI_HAS_LZCNT 1
#define HI_HAS_MOVBE 1
#define HI_HAS_OSXSAVE 1
#endif

#if defined(HI_X86_64_LEVEL) and HI_X86_64_LEVEL >= 2
#define HI_HAS_CMPXCHG16B 1
#define HI_HAS_LAHF_SAHF 1
#define HI_HAS_POPCNT 1
#define HI_HAS_SSE3 1
#define HI_HAS_SSE4_1 1
#define HI_HAS_SSE4_2 1
#define HI_HAS_SSSE3 1
#endif

#if defined(HI_X86_64_LEVEL) and HI_X86_64_LEVEL >= 1
#define HI_HAS_CMOV 1
#define HI_HAS_CX8 1
#define HI_HAS_FPU 1
#define HI_HAS_FXSR 1
#define HI_HAS_MMX 1
#define HI_HAS_OSFXSR 1
#define HI_HAS_SCE 1
#define HI_HAS_SSE 1
#define HI_HAS_SSE2 1
#endif

#if HI_COMPILER == HI_CC_CLANG
#define hi_assume(condition) __builtin_assume(not not (condition))
#define hi_force_inline __attribute__((always_inline))
#define hi_no_inline __attribute__((noinline))
#define hi_restrict __restrict__
#define hi_no_sanitize_address
#define hi_warning_push() _Pragma("warning(push)")
#define hi_warning_pop() _Pragma("warning(push)")
#define hi_warning_ignore_msvc(code)
#define hi_warning_ignore_clang(a) _Pragma(hi_stringify(clang diagnostic ignored a))

#elif HI_COMPILER == HI_CC_MSVC
#define hi_assume(condition) __assume(condition)
#define hi_force_inline __forceinline
#define hi_no_inline __declspec(noinline)
#define hi_restrict __restrict
#define hi_no_sanitize_address __declspec(no_sanitize_address)
#define hi_warning_push() _Pragma("warning( push )")
#define hi_warning_pop() _Pragma("warning( pop )")
#define hi_msvc_pragma(a) _Pragma(a)
#define hi_warning_ignore_msvc(code) _Pragma(hi_stringify(warning(disable : code)))
#define hi_warning_ignore_clang(a)

#elif HI_COMPILER == HI_CC_GCC
#define hi_assume(condition) \
    do { \
        if (!(condition)) \
            std::unreachable(); \
    } while (false)
#define hi_force_inline __attribute__((always_inline))
#define hi_no_inline __attribute__((noinline))
#define hi_restrict __restrict__
#define hi_no_sanitize_address
#define hi_warning_push() _Pragma("warning(push)")
#define hi_warning_pop() _Pragma("warning(pop)")
#define hi_msvc_pragma(a)
#define hi_warning_ignore_clang(a)
#define msvc_pragma(a)

#else
#define hi_assume(condition) static_assert(sizeof(condition) == 1)
#define hi_force_inline
#define hi_no_inline
#define hi_restrict
#define hi_no_sanitize_address
#define hi_warning_push()
#define hi_warning_pop()
#define hi_msvc_pragma(a)
#define hi_warning_ignore_clang(a)
#define msvc_pragma(a)
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

/** Debug-break.
 *
 * This function will break the application in the debugger.
 * Potentially it will start the Just-In-Time debugger if one is configured.
 * Otherwise it will continue execution.
 */
#if defined(_WIN32)
#define hi_debug_break() \
    do { \
        if (::hi::prepare_debug_break()) { \
            __debugbreak(); \
        } \
    } while (false)
#else
#error Missing implementation of hi_debug_break().
#endif

/** Debug-break and abort the application.
 *
 * This function will break the application in the debugger.
 * Potentially it will start the Just-In-Time debugger if one is configured.
 *
 * If no debugger is present than the application will be aborted with `std::terminate()`.
 * If the debugger is attached, it is allowed to continue.
 *
 * @param ... The reason why the abort is done.
 */
#if defined(_WIN32)
#define hi_assert_abort(...) \
    do { \
        ::hi::prepare_debug_break(__FILE__ ":" hi_stringify(__LINE__) ":" __VA_ARGS__); \
        __debugbreak(); \
    } while (false)
#else
#error Missing implementation of hi_assert_abort().
#endif

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

#endif
