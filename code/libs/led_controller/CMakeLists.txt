add_library(led_controller STATIC
        inc/led_controller/common_colors.h

        inc/led_controller/LedController.h
        src/LedController.cpp
)

set_target_properties(led_controller PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(led_controller PUBLIC inc)

target_link_libraries(led_controller PRIVATE drivers_interfaces)