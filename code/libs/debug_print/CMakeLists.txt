add_library(debug_print
        inc/debug_print/debug_print.h
        src/debug_print.cpp

        inc/debug_print/internal/formatting_options.h
        src/formatting_options.cpp

        inc/debug_print/internal/print_type_overloads.h
        src/print_type_overloads.cpp
)

set_target_properties(debug_print PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(debug_print PUBLIC inc)