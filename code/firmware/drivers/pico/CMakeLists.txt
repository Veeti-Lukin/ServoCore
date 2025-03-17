# Initialize the SDK which has been downloaded by the "firmware_toolchain_setup.cmake"
pico_sdk_init()

add_library(drivers_pico
        inc/drivers/BufferedAsyncUartDriver.h

        inc/drivers/PwmSliceDriver.h
        src/PwmSliceDriver.cpp

        inc/drivers/AnalogRgbLedDriver.h
        src/AnalogRgbLedDriver.cpp
)

set_target_properties(drivers_pico PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(drivers_pico PUBLIC inc)

#Publicly link the driver interfaces so hey are exposed through this library
target_link_libraries(drivers_pico PUBLIC drivers_interfaces)

target_link_libraries(drivers_pico PRIVATE
        utils
        debug_print
        assert
)

#Link privately pico sdk libraries
target_link_libraries(drivers_pico PRIVATE
        pico_stdlib
        hardware_pwm
)

target_link_libraries(drivers_pico PUBLIC drivers_interfaces)