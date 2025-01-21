# ------------------------------ Project setup ------------------------------
cmake_minimum_required(VERSION 3.20.0)

set(PICO_BOARD "pico_w") # TODO:Toolchain file or something
# Pull in SDK (must be before project)
set(PICO_SDK_FETCH_FROM_GIT ON)
include(cmake/pico_sdk_import.cmake)

project(ServoCore LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 11)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}  Version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}  Version: ${CMAKE_C_COMPILER_VERSION}")

# Initialize the SDK
pico_sdk_init()
#-----------------------------------------------------------------------------

# ------------------------------ Structure setup ------------------------------
add_subdirectory(libs)
add_subdirectory(drivers)
#-----------------------------------------------------------------------------

# ------------------------------ Firmware setup ------------------------------
add_executable(ServoCore_firmware
        src/main.cpp
)

target_include_directories(ServoCore_firmware PRIVATE inc)
target_link_libraries(ServoCore_firmware PUBLIC
        pico_stdlib

        drivers
        utils
)

pico_add_extra_outputs(ServoCore_firmware)

#-----------------------------------------------------------------------------