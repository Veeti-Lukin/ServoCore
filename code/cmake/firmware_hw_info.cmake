#File must be included before setting up pico sdk

# Placeholder until real pcb ready. Overridable from the command line so the RP2350 target can be built too:
#   cmake -DPICO_BOARD=pico2 ...
if (NOT DEFINED PICO_BOARD)
    set(PICO_BOARD "pico_w")
endif ()

if (PICO_BOARD MATCHES "^pico2")
    # RP2350: 520 kB SRAM, 4 MB flash on the Pico 2 (the RP2354A has 2 MB in-package flash)
    math(EXPR TARGET_RAM_SIZE "520 * 1024")
    math(EXPR TARGET_FLASH_SIZE "4096 * 1024")
else ()
    # RP2040
    math(EXPR TARGET_RAM_SIZE "264 * 1024")
    math(EXPR TARGET_FLASH_SIZE "2000 * 1024")
endif ()

message(STATUS "Target board: ${PICO_BOARD}")
message(STATUS "Target MCU RAM Size: ${TARGET_RAM_SIZE}")
message(STATUS "Target MCU Flash Size: ${TARGET_FLASH_SIZE}")

add_compile_definitions(TARGET_RAM_SIZE=${TARGET_RAM_SIZE})
add_compile_definitions(TARGET_FLASH_SIZE=${TARGET_FLASH_SIZE})
