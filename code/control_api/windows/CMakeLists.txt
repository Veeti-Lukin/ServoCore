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