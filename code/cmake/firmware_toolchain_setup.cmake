#File must be included before setting up pico sdk
include(cmake/firmware_hw_info.cmake)

# Pull in SDK (must be before project)
set(PICO_SDK_FETCH_FROM_GIT ON)
include(cmake/pico_sdk_import.cmake)