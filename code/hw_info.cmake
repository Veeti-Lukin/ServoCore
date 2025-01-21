#File must be included before setting up pico sdk

# Placeholder until real pcb ready
set(PICO_BOARD "pico_w")


math(EXPR TARGET_RAM_SIZE "264 * 1024")
math(EXPR TARGET_FLASH_SIZE "2000 * 1024")

message(STATUS "Target MCU RAM Size: ${TARGET_RAM_SIZE}")
message(STATUS "Target MCU Flash Size: ${TARGET_FLASH_SIZE}")

add_compile_definitions(TARGET_RAM_SIZE=${TARGET_RAM_SIZE})
add_compile_definitions(TARGET_FLASH_SIZE=${TARGET_FLASH_SIZE})