add_library(drivers_middleware
        inc/drivers/middleware/AS5600_MagneticEncoderI2cDriver.h
        src/AS5600_MagneticEncoderI2cDriver.cpp
)

set_target_properties(drivers_middleware PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(drivers_middleware PUBLIC inc)

#Publicly link the driver interfaces so that they are exposed through this library
target_link_libraries(drivers_middleware PUBLIC drivers_interfaces)

target_link_libraries(drivers_middleware PRIVATE
        utils
        debug_print
        assert
)