# Host-side tests for the Pico drivers that can be exercised against simulated hardware.
#
# This directory is added from the top-level test build, not through firmware/drivers/pico/CMakeLists.txt, because
# that file initialises the Pico SDK and can only be configured for the target. The driver sources are compiled
# here as-is against stub SDK headers in pico_sdk_stubs/, which take precedence on the include path.

set(PICO_DRIVERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(usb_driver_tests
        # Driver under test, compiled unchanged
        ${PICO_DRIVERS_DIR}/src/usb/UsbDeviceBase.cpp
        ${PICO_DRIVERS_DIR}/src/usb/UsbCdcChannelBase.cpp

        # Simulated hardware
        pico_sdk_stubs/fake_hardware.cpp

        # Tests
        usb/descriptor_plan_test.cpp
        usb/enumeration_test.cpp
        usb/cdc_channel_test.cpp
)

# Stubs first so that <hardware/...>, <pico/...> and <assert/assert.h> resolve to the fakes.
#
# utils and drivers_interfaces are header-only, but linking their targets would pull in assert -> debug_print,
# which does not build on a Linux host yet (printType(unsigned int) / printType(uint32_t) collide there). Until
# that is fixed the include directories are used directly.
target_include_directories(usb_driver_tests PRIVATE
        pico_sdk_stubs
        ${PICO_DRIVERS_DIR}/inc
        ${CMAKE_SOURCE_DIR}/common/libs/utils/inc
        ${CMAKE_SOURCE_DIR}/common/drivers/interfaces/inc
        usb
)

target_link_libraries(usb_driver_tests
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(usb_driver_tests)
