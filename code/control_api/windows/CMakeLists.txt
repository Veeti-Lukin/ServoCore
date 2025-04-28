add_library(control_api_windows
        inc/control_api/windows/internal/BufferedAsyncSerialportDriver.h
        src/BufferedAsyncSerialPortDriver.cpp

        inc/control_api/windows/Context.h
        src/Context.cpp
)

set_target_properties(control_api_windows PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(control_api_windows PUBLIC inc)

target_link_libraries(control_api_windows PUBLIC
        protocol
        utils
        control_api_template
        drivers_interfaces
)

option(SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS
        "Enable debug messages to monitor all communication trough the windows control api" off)
if (SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS)
    target_compile_definitions(control_api_windows PRIVATE SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS=1)
    message(STATUS "Control api windows comport driver debug monitor prints are enabled")
endif ()