# ------------------------------ Structure setup ------------------------------
add_subdirectory(drivers)
add_subdirectory(fw_libs)
#------------------------------------------------------------------------------


# ------------------------------ Firmware setup ------------------------------
add_executable(ServoCore_firmware
        src/main.cpp

        inc/hw_mappings.h

        inc/protocol_handlers.h
        src/protocol_handlers.cpp

        inc/interrupt_service_routines.h
        src/interrupt_service_routines.cpp
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
        protocol
)


pico_add_extra_outputs(ServoCore_firmware)

find_program(ARM_SIZE_TOOL arm-none-eabi-size REQUIRED)
add_custom_command(
        TARGET ServoCore_firmware POST_BUILD
        COMMAND python ${CMAKE_SOURCE_DIR}/tools/arm_memory_usage.py
        --size_tool_path ${ARM_SIZE_TOOL}
        --elf_file_path ${CMAKE_CURRENT_BINARY_DIR}/ServoCore_firmware.elf
        --ram_capacity ${TARGET_RAM_SIZE} # from hw_info.cmake
        --flash_capacity ${TARGET_FLASH_SIZE} # from hw_info.cmake
)
#-----------------------------------------------------------------------------