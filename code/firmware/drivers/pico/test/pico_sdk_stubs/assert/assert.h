#ifndef ASSERT_H
#define ASSERT_H
// HOST TEST STUB for common/libs/assert. A failed assertion throws so a test can observe it; the real library
// depends on debug_print, which does not currently build on Linux hosts.
#include <stdexcept>
#include <string>
#define ASSERT_WITH_MESSAGE(condition, message)                                                          \
    do {                                                                                                 \
        if (!(condition)) throw std::logic_error(std::string("assertion failed: ") + (message));         \
    } while (0)
#define ASSERT(condition) ASSERT_WITH_MESSAGE(condition, #condition)
#endif
