add_library(drivers
        inc/drivers/interfaces/SerialBufferedCommunicationInterface.h
        inc/drivers/interfaces/RgbLedInterface.h

        inc/drivers/BufferedAsyncUartDriver.h

        inc/drivers/PwmSliceDriver.h
        src/PwmSliceDriver.cpp

        inc/drivers/AnalogRgbLedDriver.h
        src/AnalogRgbLedDriver.cpp
)

set_target_properties(drivers PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(drivers PUBLIC inc)

target_link_libraries(drivers PRIVATE
        utils
        pico_stdlib
        hardware_pwm
        debug_print
        assert
)