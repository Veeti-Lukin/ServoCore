add_library(utils STATIC
        inc/utils/RingBuffer.h
        inc/utils/StaticList.h
        inc/utils/enum_helpers.h
)

set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(utils PUBLIC inc)

# Has to be public since assert is used in header because of templates
target_link_libraries(utils PUBLIC assert)


if (SERVO_CORE_BUILD_TESTS)
    add_executable(utils_tests
            test/enum_helpers_test.cpp
    )

    target_link_libraries(utils_tests
            utils
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(utils_tests)
endif ()