#include "assert/assert.h"

namespace assert {

AssertionFailedHandlerFunctionPointerType assertion_handler    = nullptr;
OnAssertFailReaction                      assert_fail_reaction = OnAssertFailReaction::break_point;

void connectAssertionFailedHandler(AssertionFailedHandlerFunctionPointerType handler) { assertion_handler = handler; }
void disconnectAssertionFailedHandler() { assertion_handler = nullptr; }

AssertionFailedHandlerFunctionPointerType getAssertionFailedHandler() { return assertion_handler; }

void setAssertionFailedReaction(OnAssertFailReaction reaction) { assert_fail_reaction = reaction; }

OnAssertFailReaction getAssertionFailedReaction() { return assert_fail_reaction; }

}  // namespace assert
