# Create an option for assertion level (default is ENABLED)
set(ServoCore_ASSERT_LEVEL 1 CACHE STRING "Assert Level (0=Disabled, 1=Enabled, 2=Verbose, 3=Debug)")

add_library(assert STATIC
        inc/assert/assert.h
        src/assert.cpp

        inc/assert/internal/on_assert_fail.h
        src/on_assert_fail.cpp
)

set_target_properties(assert PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(assert PUBLIC inc)

target_link_libraries(assert PRIVATE debug_print drivers_general)

# Define assertion levels
set(ASSERT_LEVEL_DISABLED 0)
set(ASSERT_LEVEL_ENABLED 1)
set(ASSERT_LEVEL_VERBOSE 2)
set(ASSERT_LEVEL_DEBUG 3)

target_compile_definitions(assert PUBLIC ASSERT_LEVEL_DISABLED=${ASSERT_LEVEL_DISABLED})
target_compile_definitions(assert PUBLIC ASSERT_LEVEL_ENABLED=${ASSERT_LEVEL_ENABLED})
target_compile_definitions(assert PUBLIC ASSERT_LEVEL_VERBOSE=${ASSERT_LEVEL_VERBOSE})
target_compile_definitions(assert PUBLIC ASSERT_LEVEL_DEBUG=${ASSERT_LEVEL_DEBUG})

# Ensure ServoCore_ASSERT_LEVEL is a valid value
if (NOT ServoCore_ASSERT_LEVEL EQUAL ${ASSERT_LEVEL_DISABLED} AND
        NOT ServoCore_ASSERT_LEVEL EQUAL ${ASSERT_LEVEL_ENABLED} AND
        NOT ServoCore_ASSERT_LEVEL EQUAL ${ASSERT_LEVEL_VERBOSE} AND
        NOT ServoCore_ASSERT_LEVEL EQUAL ${ASSERT_LEVEL_DEBUG})

    message(FATAL_ERROR "Invalid ServoCore_ASSERT_LEVEL value: ${ServoCore_ASSERT_LEVEL}. Must be 0 (Disabled), 1 (Enabled), 2 (Verbose) Or 3 (Debug)")
endif ()


target_compile_definitions(assert PUBLIC ServoCore_ASSERT_LEVEL=${ServoCore_ASSERT_LEVEL})


message(STATUS "Assertion level set to: ${ServoCore_ASSERT_LEVEL}")