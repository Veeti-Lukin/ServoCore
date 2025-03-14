add_library(drivers_pico
        inc/drivers/BufferedAsyncUartDriver.h

        inc/drivers/PwmSliceDriver.h
        src/PwmSliceDriver.cpp

        inc/drivers/AnalogRgbLedDriver.h
        src/AnalogRgbLedDriver.cpp
)

set_target_properties(drivers_pico PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(drivers_pico PUBLIC inc)

target_link_libraries(drivers_pico PRIVATE
        utils
        pico_stdlib
        hardware_pwm
        debug_print
        assert
)

target_link_libraries(drivers_pico PUBLIC drivers_interfaces)