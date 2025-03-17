add_library(debug_print
        inc/debug_print/debug_print.h
        src/debug_print.cpp

        inc/debug_print/internal/formatting_options.h
        src/formatting_options.cpp

        inc/debug_print/internal/print_type_overloads.h
        src/print_type_overloads.cpp
)

set_target_properties(debug_print PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(debug_print PUBLIC inc)


if (SERVO_CORE_BUILD_TESTS)
    add_executable(debug_print_tests
            test/unit_test.cpp
    )

    target_link_libraries(debug_print_tests
            debug_print
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(debug_print_tests)
endif ()
