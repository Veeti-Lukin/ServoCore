import sys
import subprocess
import argparse

TOKEN_AND_PAYLOAD_OFFSET = 6
INDENTATION = "        "


def colored_print(*text):
    COLOR_START = "\u001b[36m"  # Cyan color code
    COLOR_END = "\033[0m"

    line = "".join(text)
    print(COLOR_START + line + COLOR_END)


def calculate_percentage_usage(total_usage, max_capacity):
    """Calculate the percentage of total usage relative to maximum capacity."""
    return (total_usage / max_capacity) * 100


def calculate_total_ram_usage(memory_sections_and_usages):
    """Calculate total RAM usage (data + bss sections)."""
    return memory_sections_and_usages["data"] + memory_sections_and_usages["bss"]


def calculate_total_flash_usage(memory_sections_and_usages):
    """Calculate total flash usage (text + data sections)."""
    return memory_sections_and_usages["text"] + memory_sections_and_usages["data"]


def parse_sizes_from_stdout(stdout):
    """Parse memory section sizes from the `arm-none-eabi-size` output."""
    memory_sections_and_usages = {
        "text": 0,
        "data": 0,
        "bss": 0,
    }

    tokens = stdout.split()

    for key in memory_sections_and_usages.keys():
        key_index_in_tokens = tokens.index(key)
        memory_sections_and_usages[key] = int(tokens[key_index_in_tokens + TOKEN_AND_PAYLOAD_OFFSET])

    return memory_sections_and_usages


def main():
    parser = argparse.ArgumentParser(description="Print memory usage for Pico SDK binaries.")
    parser.add_argument("--size_tool_path", help="Path to the arm-none-eabi-size tool executable.")
    parser.add_argument("--elf_file_path", help="Path to the ELF binary file.")
    parser.add_argument("--ram_capacity", type=int, help="Total RAM capacity of the microcontroller in bytes.")
    parser.add_argument("--flash_capacity", type=int, help="Total flash capacity of the microcontroller in bytes.")

    args = parser.parse_args()

    # Call arm-none-eabi-size and capture the output
    try:
        result = subprocess.run(
            [args.size_tool_path, args.elf_file_path],
            stdout=subprocess.PIPE,
            text=True,
            check=True,
        )
    except subprocess.CalledProcessError as e:
        print(f"Error running arm-none-eabi-size: {e}")
        return

    memory_sections_and_usages = parse_sizes_from_stdout(result.stdout)

    total_flash_usage = calculate_total_flash_usage(memory_sections_and_usages)
    total_ram_usage = calculate_total_ram_usage(memory_sections_and_usages)

    flash_use_percentage = calculate_percentage_usage(total_flash_usage, args.flash_capacity)
    ram_use_percentage = calculate_percentage_usage(total_ram_usage, args.ram_capacity)

    colored_print()

    # raw memory usage
    colored_print(result.stdout)

    colored_print(f"Binary flash usage:")
    colored_print(INDENTATION, f"total: {total_flash_usage} bytes")
    colored_print(INDENTATION, f"percentage from max: {flash_use_percentage:.2f}%")

    colored_print(f"Binary RAM usage:")
    colored_print(INDENTATION, f"total: {total_ram_usage} bytes")
    colored_print(INDENTATION, f"percentage from max: {ram_use_percentage:.2f}%")
    colored_print()


if __name__ == "__main__":
    main()
