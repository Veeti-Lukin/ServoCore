add_library(math STATIC
        inc/math/crc.h
        src/crc.cpp
)

set_target_properties(math PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(math PUBLIC inc)