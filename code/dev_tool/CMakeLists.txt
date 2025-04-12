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

        inc/helpers.h
        src/helpers.cpp

        src/DeviceControlWidget.cpp
        inc/DeviceControlWidget.h
        ui/DeviceControlWidget.ui

        # ------ PARAMETER TABLE ------
        inc/parameter_table/common.h

        inc/parameter_table/ParameterTableWidget.h
        src/parameter_table/ParameterTableWidget.cpp
        ui/ParameterTableWidget.ui

        inc/parameter_table/ParameterTableModel.h
        src/parameter_table/ParameterTableModel.cpp

        inc/parameter_table/ParameterValueDelegate.h
        src/parameter_table/ParameterValueDelegate.cpp
        # ------ ---------------- ------

)

target_include_directories(ServoCore_dev_tool PRIVATE inc ui)
target_link_libraries(ServoCore_dev_tool PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::SerialPort
        control_api_windows
)

#-----------------------------------------------------------------------------