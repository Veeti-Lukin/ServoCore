# ------------------------------ Project setup ------------------------------
cmake_minimum_required(VERSION 3.20.0)

project(ServoCore LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_C_STANDARD 11)

# Pull in gtest
include(cmake/gtest_import.cmake)

message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}  Version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}  Version: ${CMAKE_C_COMPILER_VERSION}")
#-----------------------------------------------------------------------------

# ------------------------------ Structure setup ------------------------------
add_subdirectory(libs)
add_subdirectory(drivers)
#-----------------------------------------------------------------------------
