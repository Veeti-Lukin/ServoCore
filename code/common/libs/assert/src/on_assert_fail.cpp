

#include "assert/assert.h"
#include "debug_print/debug_print.h"
#include "drivers/general/break_point.h"

namespace assert::internal {

[[noreturn]] void onAssertFail(const char* expression, const char* message, const char* file, int line) {
#if ASSERT_LEVEL >= ASSERT_LEVEL_VERBOSE
    DEBUG_PRINT("\n\n\nAssertion triggered!\n");
#endif
    if (expression) {
        DEBUG_PRINT(expression);
        DEBUG_PRINT("\n");
    }
    if (message) {
        DEBUG_PRINT(message);
        DEBUG_PRINT("\n");
    }
    if (file) {
        DEBUG_PRINT("In file: %\n", file);
    }
    if (line >= 0) {
        DEBUG_PRINT("At line: %\n", line);
    }
    // wait for debug messages to be actually sent before possible breakpoint
    debug_print::flushMessages();

    if (getAssertionFailedReaction() == OnAssertFailReaction::call_assertion_handler ||
        getAssertionFailedReaction() == OnAssertFailReaction::call_assertion_handler_and_break_point) {
        if (getAssertionFailedHandler() != nullptr) getAssertionFailedHandler();
    }

    if (getAssertionFailedReaction() == OnAssertFailReaction::break_point ||
        getAssertionFailedReaction() == OnAssertFailReaction::call_assertion_handler_and_break_point) {
        while (true) drivers::general::triggerBreakpoint();
    }
}

[[noreturn]] void onAssertFail() {
    if (getAssertionFailedReaction() == OnAssertFailReaction::call_assertion_handler ||
        getAssertionFailedReaction() == OnAssertFailReaction::call_assertion_handler_and_break_point) {
        if (getAssertionFailedHandler() != nullptr) getAssertionFailedHandler();
    }

    // Given assertion handler might have sent some debug messages so
    // wait for debug messages to be actually sent before possible breakpoint
    debug_print::flushMessages();

    if (getAssertionFailedReaction() == OnAssertFailReaction::break_point ||
        getAssertionFailedReaction() == OnAssertFailReaction::call_assertion_handler_and_break_point) {
        while (true) drivers::general::triggerBreakpoint();
    }
}

}  // namespace assert::internal