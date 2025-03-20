add_library(drivers_general
        inc/drivers/general/break_point.h
)

set_target_properties(drivers_general PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(drivers_general PUBLIC inc)
