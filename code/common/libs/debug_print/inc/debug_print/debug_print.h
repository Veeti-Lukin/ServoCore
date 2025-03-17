#ifndef LIBS_DEBUG_PRINT_DEBUG_PRINT_H
#define LIBS_DEBUG_PRINT_DEBUG_PRINT_H

#include <cstdint>

#include "debug_print/internal/formatting_options.h"
#include "debug_print/internal/print_type_overloads.h"

namespace debug_print {

inline constexpr char K_PLACEHOLDER_FORMAT_CHAR = '%';

using PutCharFunctionPointerType                = void (*)(char c);
using FlushFunctionPointerType                  = void   (*)();

/**
 * @brief Connect functions for sending a single character and flushing output.
 *
 * This function allows users to specify a function for outputting characters, enabling
 * custom implementations for specific peripherals (e.g., UART, USB, or display devices).
 * Additionally, a flush function can be provided to ensure all buffered output is sent.
 *
 * @param put_char_function A function pointer to a putchar function for outputting characters.
 * @param flush_function A function pointer to a flush function that ensures all pending output is transmitted
 * (optional).
 */
void connectPutCharAndFlushFunctions(PutCharFunctionPointerType put_char_function,
                                     FlushFunctionPointerType   flush_function);

/**
 * @brief Disconnect the currently connected putchar function.
 *
 * Sets the internal putchar function pointer to `nullptr`, effectively disabling character output.
 */
void disconnectPutCharFunction();

/**
 * @brief Disconnect the currently connected flush function.
 *
 * Sets the internal flush function pointer to `nullptr`, effectively disabling flushing.
 */
void disconnectFlushFunction();

/**
 * @brief Retrieve the currently connected putchar function.
 *
 * @return The function pointer to the connected putchar function, or `nullptr` if none is connected.
 */
PutCharFunctionPointerType getPutCharFunction();

/**
 * @brief Flush any buffered output messages using the connected flush function.
 *
 * If a flush function is connected, this function will call it to ensure all output is sent.
 */
void flushMessages();

/**
 * @brief Print format string using the connected PutChar function.
 *
 * Base case for variadic template print function. Can be called aswell if no argument are given at all.
 *
 * Processes a format string without additional arguments, iterating through and printing
 * each character. If a placeholder is encountered, it indicates a formatting error due
 * to missing arguments.
 *
 * @param format_string The format string to process.
 */
void printFormat(const char* format_string);

/**
 * @brief Print format string using the connected PutChar function.
 *
 * Variadic template function for formatted printing.
 * Parses a format string and substitutes placeholders with provided arguments. Each argument
 * is processed based on its type automatically using overload functions (check internal/print_type_overloads).
 *
 * USGAE: \n
 *  debug_print::PrintFormat("Int: %, binary: %b, hex: %x, char: %, string: %, bool: %", 42, 3, 5, 'c', "test", false)
 *
 *
 * SUPPORTED FORMATTING OPTIONS:
 *  - b : decimal as binary
 *  - h : decimal as hex
 *  - 0-9 : amount of decimals for floating points and amount of digits for decimals but does not truncate
 *
 * NOTES:
 *  - The escape/format character can only be brinted by formatting it as a character
 *  - Floating points maximum is the same as for int64_t
 *
 * @tparam T The type of the first argument.
 * @tparam Args The types of the remaining arguments.
 * @param format_string The format string containing placeholders.
 * @param value The first argument to format in the format string.
 * @param args The remaining arguments to process.
 */
template <typename T, typename... Args>
void printFormat(const char* format_string, T value, Args... args);
}  // namespace debug_print

/**
 * @brief Print format string using the connected PutChar function.
 *
 * For more detail see: {@link  printFormat @endlink}
 */
#define DEBUG_PRINT(format_string, ...) debug_print::printFormat(format_string, ##__VA_ARGS__)

//
//
//
//
//
//
//
//
//

/// ------------------------ TEMPLATE DEFINITIONS --------------------------------------

namespace debug_print {

// Variadic template for handling multiple arguments
template <typename T, typename... Args>
void printFormat(const char* format_string, T value, Args... args) {
    if (!getPutCharFunction()) return;

    // Every time function recurses reset the formatting options for the next argument
    internal::resetFormattingOptions();

    for (const char* token_ptr = format_string; *token_ptr != '\0'; ++token_ptr) {
        if (*token_ptr == K_PLACEHOLDER_FORMAT_CHAR) {
            token_ptr++;  // Ignore the escape character and move to next one

            // if true this character is a formatting option and must be skipped and not printed after handling it in
            if (internal::tryParseFormattingOptions(*token_ptr)) token_ptr++;

            // Print the first argument using overload function depending of the type of this
            internal::printType(value);

            // Recursive call for remaining arguments or if no arguments left calls the base case of this function
            printFormat(token_ptr, args...);
            return;
        }

        // some terminals except carriage return before new line character
        // if only new line character is put inside the format string print carriage return before it
        if (*token_ptr == '\n' && *(token_ptr - 1) != '\r') {
            getPutCharFunction()('\r');
        }
        // The base case while iterating the format string is just to print the character
        getPutCharFunction()(*token_ptr);
    }
}

}  // namespace debug_print

#endif  // LIBS_DEBUG_PRINT_DEBUG_PRINT_H
