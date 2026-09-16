cmake_minimum_required(VERSION 3.25.0)

# --------------------------------- Options ---------------------------------
option(SERVO_CORE_BUILD_TESTS "Build tests" off)
option(SERVO_CORE_FIRMWARE_BUILD "Firmware build" off)
#----------------------------------------------------------------------------


# ------------------------------ Project setup ------------------------------
# Toolchain setup must be done before the project calls
# On host platform this toolchain can be determined automatically for other type of builds
if (SERVO_CORE_FIRMWARE_BUILD)
    include(cmake/firmware_toolchain_setup.cmake)
endif ()

project(ServoCore LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 11)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}  Version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}  Version: ${CMAKE_C_COMPILER_VERSION}")

if (SERVO_CORE_BUILD_TESTS)
    include(cmake/gtest_import.cmake)
endif ()

# --------- Experimental C++ 26 reflection support setup ----------
include(CheckCXXCompilerFlag)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    check_cxx_compiler_flag(-freflection _SUPPORTS_FREFLECTION_FLAG)
    if (_SUPPORTS_FREFLECTION_FLAG)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-freflection>)
        # try_compile does not inherit add_compile_options, so the check below has to be told
        # about the flag separately
        set(CMAKE_REQUIRED_FLAGS -freflection)
    endif ()
endif ()
# TODO: Add clang or other similar here too

include(CheckCXXSourceCompiles)

set(_REFLECTION_TEST_SOURCE "
#ifndef __cpp_impl_reflection
#error no static reflection
#endif
int main() {}
")
check_cxx_source_compiles("${_REFLECTION_TEST_SOURCE}" _REFLECTION_TEST_COMPILES)
# Cleared right after the check that needs it, so later check_ calls - including the ones inside
# dependencies - are not compiled with the flag
unset(CMAKE_REQUIRED_FLAGS)

if (NOT _REFLECTION_TEST_COMPILES)
    message(FATAL_ERROR
            "${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}) does not "
            "define __cpp_impl_reflection, A compiler with C++26 static reflection support is required")
endif ()


#-----------------------------------------------------------------------------


# ------------------------------ Structure setup ------------------------------
if (SERVO_CORE_FIRMWARE_BUILD)
    add_subdirectory(firmware)
    add_subdirectory(common)
else ()
    add_subdirectory(common)
    add_subdirectory(control_api)
    add_subdirectory(dev_tool) # after control_api, it checks for control_api_windows
endif ()
#-----------------------------------------------------------------------------
