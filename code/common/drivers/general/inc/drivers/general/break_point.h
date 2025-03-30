#ifndef BREAK_POINT_H
#define BREAK_POINT_H

namespace drivers::general {

/**
 * @brief Triggers a breakpoint for debugging purposes.
 *
 * This function issues an architecture-specific breakpoint instruction,
 * causing the debugger to halt execution at the call site.
 * If the architecture is unsupported, the function defaults to a no-op.
 *
 * Supported architectures:
 * - ARM (RP2040, STM32, etc.): Uses `bkpt #0`
 * - x86/x86_64: Uses `int $3`
 * - Windows (MSVC): Uses `__debugbreak()`
 * - macOS/iOS (Clang): Uses `std::raise(SIGTRAP)`
 *
 * @note Ensure that a debugger is attached; otherwise, behavior may be undefined.
 */
inline void volatile triggerBreakpoint() {
#if defined(__GNUC__) || defined(__clang__)  // GCC/Clang
#if defined(__arm__) || defined(__thumb__)   // ARM (RP2040, STM32, etc.)
    __asm volatile("bkpt #0" : : : "memory");
#elif defined(__x86_64__) || defined(__i386__)  // x86/x86_64
    __asm volatile("int $3" : : : "memory");
#else
    ((void)0)  // Unsupported arch
#endif

#elif defined(_MSC_VER)  // MSVC (Windows)
    __debugbreak()

#elif defined(__APPLE__) && defined(__MACH__)  // macOS/iOS (Clang)
#include <signal>
    std::raise(SIGTRAP)

#else
    ((void)0)  // Default to no-op

#endif
}

}  // namespace drivers::general

#endif  // BREAK_POINT_H
