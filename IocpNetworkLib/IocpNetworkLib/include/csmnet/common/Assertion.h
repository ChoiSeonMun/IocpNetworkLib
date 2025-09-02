#pragma once

#include <string_view>
#include <spdlog/spdlog.h>
#include <format>

#include <source_location>

// Platform-specific debug break
#if defined(_MSC_VER)
    #define CSM_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    #define CSM_DEBUG_BREAK() __builtin_trap()
#else
    #include <csignal>
    #define CSM_DEBUG_BREAK() std::raise(SIGTRAP)
#endif

// Enable assertions only in debug builds
#if (defined(_DEBUG) || !defined(NDEBUG))
    #define CSM_ENABLE_ASSERTS
#endif

#ifdef CSM_ENABLE_ASSERTS
    namespace csmnet::internal
    {
        // Handler for CSM_ASSERT (no custom message)
        inline void handleAssertionFailure(
            std::string_view expression,
            std::source_location location = std::source_location::current())
        {
            spdlog::critical("Assertion Failed: {} @ [{}] {}:{}",
                expression,
                location.function_name(),
                location.file_name(),
                location.line());
        }

        // Handler for CSM_ASSERT_MSG (with custom message)
        template<typename... Args>
        void handleAssertionFailure(
            std::string_view expression,
            std::source_location location,
            const std::format_string<Args...> fmt, 
            Args&&... args)
        {
            spdlog::critical("Assertion Failed({}): {} @ [{}] {}:{}",
                std::vformat(fmt.get(), std::make_format_args(args...)),
                expression,
                location.function_name(),
                location.file_name(),
                location.line());
        }
    }

    // Use this for simple assertions
    #define CSM_ASSERT(expr) \
        do { \
            if (!(expr)) { \
                csmnet::internal::handleAssertionFailure(#expr); \
                CSM_DEBUG_BREAK(); \
            } \
        } while (false)

    // Use this for assertions with a descriptive message
    // ##__VA_ARGS__ is a GCC/Clang extension that handles trailing commas gracefully. MSVC also supports it.
    #define CSM_ASSERT_MSG(expr, msg, ...) \
        do { \
            if (!(expr)) { \
                csmnet::internal::handleAssertionFailure(#expr, std::source_location::current(), msg, ##__VA_ARGS__); \
                CSM_DEBUG_BREAK(); \
            } \
        } while (false)

#else
    // In release builds, assertions compile to nothing
    #define CSM_ASSERT(expr) ((void)0)
    #define CSM_ASSERT_MSG(expr, msg, ...) ((void)0)
#endif
