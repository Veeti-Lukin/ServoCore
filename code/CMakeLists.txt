# --------------------------------- Options ---------------------------------
option(SERVO_CORE_BUILD_TESTS "Build tests" off)
option(SERVO_CORE_FIRMWARE_BUILD "Firmware build" off)
#----------------------------------------------------------------------------


# ------------------------------ Project setup ------------------------------
cmake_minimum_required(VERSION 3.20.0)

# Toolchain setup must be done before the project calls
# On host platform this toolchain can be determined automatically for other type of builds
if (SERVO_CORE_FIRMWARE_BUILD)
    include(cmake/firmware_toolchain_setup.cmake)
endif ()

project(ServoCore LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 11)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}  Version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}  Version: ${CMAKE_C_COMPILER_VERSION}")

if (SERVO_CORE_BUILD_TESTS)
    include(cmake/gtest_import.cmake)
endif ()
#-----------------------------------------------------------------------------


# ------------------------------ Structure setup ------------------------------
if (SERVO_CORE_FIRMWARE_BUILD)
    add_subdirectory(firmware)
    add_subdirectory(common)
else ()
    add_subdirectory(common)
    add_subdirectory(dev_tool)
endif ()
#-----------------------------------------------------------------------------