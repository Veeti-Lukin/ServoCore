# --------------------------------- Options ---------------------------------
option(SERVO_CORE_BUILD_TESTS "Build tests on host platform" off)
#----------------------------------------------------------------------------


# ------------------------------ Test setup ------------------------------
if (SERVO_CORE_BUILD_TESTS)
    include(cmake/TestBuildCMakeLists.cmake)
    return() # Skip all the other project setup in this file
endif ()
#-----------------------------------------------------------------------------


# ------------------------------ Project setup ------------------------------
cmake_minimum_required(VERSION 3.20.0)

#File must be included before setting up pico sdk
include(hw_info.cmake)

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

        inc/hw_mappings.h

        inc/serial_communication_framework_op_code_handlers.h
        src/serial_communication_framework_op_code_handlers.cpp
)

target_include_directories(ServoCore_firmware PRIVATE inc)
target_link_libraries(ServoCore_firmware PUBLIC
        pico_stdlib
        hardware_pwm

        drivers_pico
        utils
        debug_print
        led_controller
        parameter_system
        serial_communication_framework
        assert
)

pico_add_extra_outputs(ServoCore_firmware)

find_program(ARM_SIZE_TOOL arm-none-eabi-size REQUIRED)
add_custom_command(
        TARGET ServoCore_firmware POST_BUILD
        COMMAND python ${CMAKE_SOURCE_DIR}/tools/arm_memory_usage.py
        --size_tool_path ${ARM_SIZE_TOOL}
        --elf_file_path ${CMAKE_BINARY_DIR}/ServoCore_firmware.elf
        --ram_capacity ${TARGET_RAM_SIZE} # from hw_info.cmake
        --flash_capacity ${TARGET_FLASH_SIZE} # from hw_info.cmake
)
#-----------------------------------------------------------------------------