# Code

## About

Software for the ServoCore project. See the [root README](../README.md) for a project overview.

## Building

### Requirements

- **Firmware builds only:** [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) — choose the bare-metal target (`arm-none-eabi`) for your host platform and add its `bin/` to `PATH`.
- **Dev tool (optional):** Qt6 (Core, Widgets, SerialPort). The dev tool is built as part of the host build when Qt6 is found — if not found it is skipped with a warning. Qt6 must be locatable by CMake; either add Qt's `bin/` to `PATH` or pass `-DCMAKE_PREFIX_PATH=<Qt install>/lib/cmake` at configure time (e.g. `C:\Qt\6.8.2\mingw_64\lib\cmake` on Windows).

External dependencies (Pico SDK, GTest) are fetched automatically at configure time via `FetchContent`.

### Building with CMake

All commands run from the `code/` directory.

**Host build** (control API + libraries, no ARM toolchain required):
```bash
cmake -B build
cmake --build build
```

**Firmware build:**
```bash
cmake -B build_fw -DSERVO_CORE_FIRMWARE_BUILD=ON
cmake --build build_fw
```

**Tests:**
```bash
cmake -B build_test -DSERVO_CORE_BUILD_TESTS=ON
cmake --build build_test
./build_test/debug_print_tests
```

### Building with CLion

Open the `code/` directory as a CLion project. When prompted to configure CMake profiles (or via *File → Settings → Build, Execution, Deployment → CMake*), create profiles for the builds you need:

**Host profile:**
- Name: `Host-Release` / `Host-Debug`
- Build type: `Release` / `Debug`
- Generator: `Ninja`
- CMake options: `-DCMAKE_PREFIX_PATH=C:\Qt\6.8.2\mingw_64\lib\cmake` *(adjust to your Qt install path)*

**Firmware profile:**
- Name: `Firmware-Release` / `Firmware-Debug`
- Build type: `Release` / `Debug`
- Toolchain: set to the ARM GNU toolchain
- Generator: `Ninja`
- CMake options: `-DSERVO_CORE_FIRMWARE_BUILD=ON`

#### Debugging firmware with J-Link

Add a run configuration for the embedded GDB server (*pull-down next to the build button → Edit Configurations → + → Embedded GDB Server*):

| Field | Value |
|-------|-------|
| Name | `J-Link` |
| Target remote args | `localhost:2331` |
| GDB Server | path to `JLinkGDBServerCLExe` (e.g. `C:\Program Files\SEGGER\JLink\JLinkGDBServerCL.exe`) |
| GDB Server args | `-if SWD -device RP2040_M0_0` |

Running this configuration will flash and attach the debugger via J-Link.

SVD files for peripheral register visualization are in `tools/`:

| File | Use |
|------|-----|
| `tools/RP2040.svd` | Current development target |
| `tools/RP2350.svd` | Final production target |

Point the run configuration's SVD path to the appropriate file to get hardware register views in the debugger.

### CMake options

| Option | Default | Purpose |
|--------|---------|---------|
| `SERVO_CORE_FIRMWARE_BUILD` | OFF | Switch to firmware/ARM build |
| `SERVO_CORE_BUILD_TESTS` | OFF | Enable GTest unit tests |
| `ServoCore_ASSERT_LEVEL` | — | Assertion verbosity (0 = disabled, 3 = most verbose) |
| `SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS` | OFF | Print all bytes passing through the serial driver |
| `SERVO_CORE_DISABLE_SERIAL_COMMUNICATION_FRAMEWORK_TIMEOUTS` | OFF | Disable packet timeouts (debugging aid) |

---

## Structure

```
code/
├── common/             # Shared between firmware and host
│   ├── drivers/
│   │   └── interfaces/ # Abstract driver interfaces (serial, LED, timer, clock)
│   ├── libs/
│   │   ├── serial_communication_framework/
│   │   ├── parameter_system/
│   │   ├── assert/
│   │   ├── debug_print/
│   │   ├── utils/      # RingBuffer, StaticList
│   │   └── math/       # CRC
│   └── protocol/       # Concrete command definitions
├── firmware/           # Embedded application (RP2040 / RP2350)
├── control_api/
│   ├── template/       # Platform-agnostic master layer
│   ├── windows/        # Windows implementation
│   └── python/         # Python bindings (planned)
└── dev_tool/           # Qt6 GUI for device control and parameter inspection
```

---

## Architecture

### Serial Communication Framework

`common/libs/serial_communication_framework/`

A binary packet protocol that runs over serial. The framework defines a type-safe command model — each command pairs a request type and a response type with an operation code, enforced at compile time.

There are two sides: a **master** (host) that sends commands and waits for responses, and a **slave** (firmware) that receives commands, dispatches them to registered handlers, and responds. Both sides validate packets using a two-level CRC (header + payload).

### Protocol

`common/protocol/`

The concrete set of commands the device supports — ping, parameter read/write, metadata queries, motor control, etc. Built on top of the serial communication framework.

### Parameter System

`common/libs/parameter_system/`

A typed parameter system that lets the firmware expose device state and configuration to the host over the protocol. Parameters have a value type (e.g. `uint8`, `float`, `int32`), a name, and a read/write access level. There are three categories:

- **Saved** — persists across reboots
- **Runtime** — resets on reboot
- **Signal** — read-only, device-generated (sensor data etc.)

The host can enumerate all parameters, query their metadata, and read or write their values at runtime.

### Control API

`control_api/`

The library used to communicate with a ServoCore device. The design is split into a platform-agnostic template layer and platform-specific implementations (currently Windows, Python bindings planned). The template layer is intentionally portable — it can run on a desktop host or be compiled for a microcontroller, so another MCU can act as the master and control the ServoCore board.

### Firmware

`firmware/`

The embedded application running on the microcontroller. Handles hardware initialization, drives the motor, reads the encoder, and processes incoming protocol commands.

Driver implementations are separated from the rest of the firmware via interfaces (`common/drivers/interfaces/`), keeping hardware-specific code isolated and the rest of the codebase portable.

---

## Conventions

- `K_` prefix for compile-time constants.
- `std::span<uint8_t>` for all buffer passing (zero-copy).
- No heap allocation in firmware — use `StaticList`, `RingBuffer`, and fixed-size arrays.
- Namespaces mirror the directory structure.

### Naming

| Thing | Convention | Example |
|-------|-----------|---------|
| Classes & types | `PascalCase` | `SlaveHandler` |
| Interfaces | `PascalCase` + `Interface` suffix | `RgbLedInterface` |
| Functions & methods | `camelCase` | `callMyGoodFunction` |
| Variables | `snake_case` | `my_variable` |
| Class members | `snake_case` + `_` suffix | `my_member_` |
| Enum members | `snake_case` | `timed_out` |
| Files (single class) | matches class name | `SlaveHandler.cpp` |
| Files (broader/utility) | `snake_case` | `serialize_deserialize.cpp` |
