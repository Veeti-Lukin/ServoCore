if (SERVO_CORE_BUILD_TESTS)
    add_subdirectory(debug_print)
    return()
endif ()

add_subdirectory(utils)
add_subdirectory(debug_print)
add_subdirectory(led_controller)
