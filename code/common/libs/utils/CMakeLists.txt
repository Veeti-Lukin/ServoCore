add_library(utils STATIC
        inc/utils/RingBuffer.h
        inc/utils/StaticList.h
)

set_target_properties(utils PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(utils PUBLIC inc)

# Has to be public since assert is used in header because of templates
target_link_libraries(utils PUBLIC assert)