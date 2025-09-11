# Initialize the SDK which has been downloaded by the "firmware_toolchain_setup.cmake"
pico_sdk_init()

add_library(drivers_pico
        inc/drivers/BufferedAsyncUartDriver.h

        inc/drivers/PwmSliceDriver.h
        src/PwmSliceDriver.cpp

        inc/drivers/AnalogRgbLedDriver.h
        src/AnalogRgbLedDriver.cpp

        inc/drivers/TimerDriver.h
        src/TimerDriver.cpp

        inc/drivers/SysClockDriver.h
        src/SysClockDriver.cpp

        inc/drivers/I2CMasterDriver.h
        src/I2CMasterDriver.cpp
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

#Link publicly pico sdk libraries
target_link_libraries(drivers_pico PUBLIC
        pico_stdlib
        hardware_pwm
        hardware_i2c
)

target_link_libraries(drivers_pico PUBLIC drivers_interfaces)