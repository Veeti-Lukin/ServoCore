add_library(drivers
        inc/drivers/interfaces/SerialBufferedCommunicationInterface.h

        inc/drivers/BufferedAsyncUartDriver.h
)

set_target_properties(drivers PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(drivers PUBLIC inc)
target_link_libraries(drivers PRIVATE utils pico_stdlib)