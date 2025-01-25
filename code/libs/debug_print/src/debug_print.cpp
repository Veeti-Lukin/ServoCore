#include "debug_print/debug_print.h"

namespace debug_print {

extern PutCharFunctionPointerType put_char_function_pointer = nullptr;

void connectPutCharFunction(PutCharFunctionPointerType put_char_function) {
    put_char_function_pointer = put_char_function;
}

void disconnectPutCharFunction() { put_char_function_pointer = nullptr; }

PutCharFunctionPointerType getPutCharFunction() { return put_char_function_pointer; }

// Base case for printf
void printFormat(const char* format_string) {
    if (!getPutCharFunction()) return;

    internal::resetFormattingOptions();

    for (const char* token_ptr = format_string; *token_ptr != '\0'; ++token_ptr) {
        // if the character before this or character after this token
        if (*token_ptr == K_PLACEHOLDER_FORMAT_CHAR) {
            // ERROR NO ARGUMENTS (LEFT) OR JUST PRINTING WITHOUT ARGUMENTS BUT ESCAPE SYMBOL FOUND
            internal::printType("[No argument?]");
            // SKip the placeholder formatting character and move to next character
            token_ptr++;
            // if true this character is a formatting option and must be skipped and not printed after handling it in
            if (internal::tryParseFormattingOptions(*token_ptr)) token_ptr++;
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
