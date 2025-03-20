add_library(utils STATIC
        inc/utils/RingBuffer.h
)

set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(utils PUBLIC inc)