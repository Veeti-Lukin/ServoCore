if (SERVO_CORE_BUILD_TESTS)
    add_subdirectory(interfaces)
    return() # Skip all the other project setup in this file
endif ()

add_subdirectory(interfaces)
add_subdirectory(pico)