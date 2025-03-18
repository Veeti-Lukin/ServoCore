# ------------------------------ Structure setup ------------------------------
#-----------------------------------------------------------------------------


# ----------------------------- Application setup ------------------------------
include(../cmake/qt_import.cmake)

if (NOT QT_FOUND)
    message(WARNING "DevTool cant be built QT not found")
    return()
endif ()

set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

add_executable(ServoCore_dev_tool
        src/main.cpp

        inc/MainWindow.h
        src/MainWindow.cpp
        ui/MainWindow.ui
)

target_include_directories(ServoCore_dev_tool PRIVATE inc ui)
target_link_libraries(ServoCore_dev_tool PUBLIC
        Qt6::Core Qt6::Widgets
)

#-----------------------------------------------------------------------------