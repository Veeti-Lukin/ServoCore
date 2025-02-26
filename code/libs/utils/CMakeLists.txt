add_library(utils STATIC
        inc/utils/RingBuffer.h
        
        inc/utils/break_point.h
)

set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(utils PUBLIC inc)