# ------------------------------ Structure setup ------------------------------
#-----------------------------------------------------------------------------


# ----------------------------- Application setup ------------------------------
# The dev tool talks to devices through the Windows control API backend, which only exists for Windows targets
if (NOT TARGET control_api_windows)
    message(STATUS "Dev tool skipped - it needs the Windows control API backend")
    return()
endif ()

include(../cmake/qt_import.cmake)

if (NOT QT_FOUND)
    message(WARNING "DevTool cant be built QT not found")
    return()
endif ()

set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui")

set(_SERVO_CORE_DEVTOOL_TARGET ServoCore_dev_tool)
add_executable(${_SERVO_CORE_DEVTOOL_TARGET}
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

target_include_directories(${_SERVO_CORE_DEVTOOL_TARGET} PRIVATE inc ui)
target_link_libraries(${_SERVO_CORE_DEVTOOL_TARGET} PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::SerialPort

        control_api_windows
)
#-----------------------------------------------------------------------------


# ------------------------------ Windows deployment ----------------------------
# Windows searches an executable's own directory for DLLs before it searches PATH, so keeping the
# right libraries next to the exe makes the dev tool independent of whatever happens to be on PATH.
#
# --no-compiler-runtime makes windeployqt to not copy compiler CPP runtime libraries
# (for example on GCC libstdc++) in the build directory from Qt installation.
# Qt is only used as a prebuilt binary, and we are not using compilers that Qt packages inside to
# ensure C++ 26 compatibility.
# Dll mismatch would cause ABI incompatibility and would cause exe to crash.
#
# --no-translations only skips Qt's .qm localisation files.

if (WIN32)
    # Qt6::windeployqt is an imported tool target that comes with Qt6::Core. Using the target rather
    # than a path lets CMake resolve it for whichever build configuration is active
    if (NOT TARGET Qt6::windeployqt)
        message(FATAL_ERROR "Qt6::windeployqt is missing from this Qt installation - the dev tool "
                "cannot be deployed, and would only run where Qt happens to be on PATH")
    endif ()

    # The compiler runtime goes first, while the output directory still holds nothing but the
    # executable. The scan below resolves only what it can find in the compiler's own bin directory,
    # so running it before windeployqt keeps Qt's libraries out of its way entirely.
    get_filename_component(_compiler_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    add_custom_command(TARGET ${_SERVO_CORE_DEVTOOL_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            -DEXECUTABLE=$<TARGET_FILE:${_SERVO_CORE_DEVTOOL_TARGET}>
            -DDESTINATION=$<TARGET_FILE_DIR:${_SERVO_CORE_DEVTOOL_TARGET}>
            -DSEARCH_DIRS=${_compiler_bin_dir}
            -P ${CMAKE_SOURCE_DIR}/cmake/deploy_runtime_dependencies.cmake
            COMMENT "Deploying the compiler runtime next to the dev tool"
    )

    add_custom_command(TARGET ${_SERVO_CORE_DEVTOOL_TARGET} POST_BUILD
            COMMAND Qt6::windeployqt --no-compiler-runtime --no-translations
            "$<TARGET_FILE:${_SERVO_CORE_DEVTOOL_TARGET}>"
            COMMENT "Deploying Qt libraries next to the dev tool"
    )
endif ()
#-----------------------------------------------------------------------------