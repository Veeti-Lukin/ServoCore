#ifndef ON_ASSERT_FAIL_H
#define ON_ASSERT_FAIL_H

namespace assert::internal {

void onAssertFail(const char* expression, const char* message, const char* file, int line);
void onAssertFail();

}  // namespace assert::internal

#endif  // ON_ASSERT_FAIL_H