#ifndef ASSERT_H
#define ASSERT_H

#include "assert/internal/on_assert_fail.h"

namespace assert {

#if ServoCore_ASSERT_LEVEL == ASSERT_LEVEL_DISABLED
#define ASSERT(expression)                        // No-op
#define ASSERT_WITH_MESSAGE(expression, message)  // No-op

#elif ServoCore_ASSERT_LEVEL == ASSERT_LEVEL_ENABLED
#define ASSERT(expression) \
    if (!(expression)) assert::internal::onAssertFail()
#define ASSERT_WITH_MESSAGE(expression, message) \
    if (!(expression)) assert::internal::onAssertFail()

#elif ServoCore_ASSERT_LEVEL == ASSERT_LEVEL_VERBOSE
#define ASSERT(expression) \
    if (!(expression)) assert::internal::onAssertFail(#expression, nullptr, nullptr, -1)
#define ASSERT_WITH_MESSAGE(expression, message) \
    if (!(expression)) assert::internal::onAssertFail(#expression, message, nullptr, -1)

#elif ServoCore_ASSERT_LEVEL == ASSERT_LEVEL_DEBUG
#define ASSERT(expression) \
    if (!(expression)) assert::internal::onAssertFail(#expression, nullptr, __FILE__, __LINE__)
#define ASSERT_WITH_MESSAGE(expression, message) \
    if (!(expression)) assert::internal::onAssertFail(#expression, message, __FILE__, __LINE__)

#endif

enum class OnAssertFailReaction {
    call_assertion_handler,
    break_point,  // DEFAULT
    call_assertion_handler_and_break_point,
};

using AssertionFailedHandlerFunctionPointerType = void (*)();
void connectAssertionFailedHandler(AssertionFailedHandlerFunctionPointerType handler);

void disconnectAssertionFailedHandler();

AssertionFailedHandlerFunctionPointerType getAssertionFailedHandler();

void                 setAssertionFailedReaction(OnAssertFailReaction reaction);
OnAssertFailReaction getAssertionFailedReaction();

}  // namespace assert

#endif  // ASSERT_H
