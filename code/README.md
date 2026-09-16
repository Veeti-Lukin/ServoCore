# Code

## About

Software for the ServoCore project. See the [root README](../README.md) for a project overview.

## Terminology

| Term         | Meaning                                                                                                                            |
|--------------|------------------------------------------------------------------------------------------------------------------------------------|
| **device**   | A ServoCore board running the firmware. The slave on the bus.                                                                      |
| **consumer** | Whatever drives the bus and talks to ServoCore devices through `control_api`. Either a PC application, or another microcontroller. |
| **dev tool** | The Qt GUI in `dev_tool/`.                                                                                                         |
| **host**     | The computer you build on (Windows, Linux).                                                                                        |
| **target**   | The platform a built binary runs on: your PC, ServoCore MCU, or another MCU.                                                       |

## Building

### Toolchain requirements

The project targets **C++26 standard** and has been tested with GCC 16 (Tho other compilers and newer versions might
work in the future)

Requirements come in sets - pick the one matching what you are building.
Common requirements between all sets are:

| Component | Version    | Where to get it                                                                                                      | After installing                                                                                        |
|-----------|------------|----------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------|
| CMake     | \>= 3.25   | [cmake.org/download](https://cmake.org/download/) - **pick one that matches your host platform**                     | Tick **Add CMake to the system PATH** in the installer. Check with `cmake --version`.                   |
| Ninja     | any recent | [ninja-build releases](https://github.com/ninja-build/ninja/releases) - **pick one that matches your host platform** | Just one `ninja.exe`. Extract it somewhere and add that folder to `PATH`. Check with `ninja --version`. |
| Python    | 3.x        | [python.org](https://www.python.org/downloads/) - **pick one that matches your host platform**                       | Tick **Add python.exe to PATH** in the installer. Check with `python --version`.                        |
| Git       | any recent | [git-scm.com/downloads](https://git-scm.com/downloads) - **pick one that matches your host platform**                | The installer's default adds it to `PATH`. Check with `git --version`.                                  |

> CLion IDE bundles its own copy of all of these (**EXCEPT PYTHON AND GIT!**), so building only from the IDE needs none
> of these downloaded separately. The command line builds further down do.
> See [Building with CLion](#building-with-clion).

External code dependencies (Pico SDK, GTest, etc...) are fetched automatically at configure time via CMake
`FetchContent`.

#### **Set 1 - `control_api` in your own consumer application**

Needs nothing but a C++26 compiler for the [target](#terminology) that [consumer](#terminology) runs
on, which may be a PC or another microcontroller.

| Component                                              | Version                                          | Where to get it                     |
|--------------------------------------------------------|--------------------------------------------------|-------------------------------------|
| C++26 (cross) compiler for your [target](#terminology) | Any compiler supporting C++26 static reflections | Depends on the target you build for |

#### **Set 2 - Dev tool**

Builds the [dev tool](#terminology), a [consumer](#terminology) that runs on your [host](#terminology).

| Component                                    | Version                                          | Where to get it                                               |
|----------------------------------------------|--------------------------------------------------|---------------------------------------------------------------|
| C++26 compiler for your [host](#terminology) | Any compiler supporting C++26 static reflections | Depends on your host                                          |
| Qt                                           | Qt 6.10+, Core / Widgets / SerialPort            | [Qt online installer](https://www.qt.io/download-open-source) |

In the Qt installer, pick the kit matching your compiler's ABI: MinGW GCC needs a `mingw_64` kit, MSVC an
`msvc` kit. A wrong kit is not caught at configure time. An MSVC kit with GCC configures and compiles
fine, and only fails at the final link.

> Note that the project will be built with a different compiler than the one Qt's prebuilt binaries are built with.
> That mixes safely because only Qt types cross the DLL boundary.

#### **Set 3 - Firmware**

Builds the firmware for the ServoCore [device](#terminology). The firmware itself is cross compiled with
`arm-none-eabi`.
The Pico SDK also builds picotool for your [host](#terminology) during the build, using the first host compiler on
`PATH`, which is why a host compiler is needed too.

| Component                                  | Version                                                                    | Where to get it                                                                        |
|--------------------------------------------|----------------------------------------------------------------------------|----------------------------------------------------------------------------------------|
| Firmware compiler                          | Any `arm-none-eabi` compiler supporting C++26 static reflections (GCC 16+) | [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) |
| [Host](#terminology) compiler for picotool | Any recent C/C++ compiler, on `PATH`                                       | Depends on your host                                                                   |

On Windows, the host compiler's `bin` must come before any other folder on `PATH` that ships an older
`libstdc++-6.dll` (Qt's `bin`, for example), or the SDK-built picotool crashes.

Optional components:

| Component         | Version             | Where to get it                                   | Needed for                      |
|-------------------|---------------------|---------------------------------------------------|---------------------------------|
| Firmware debugger | `arm-none-eabi-gdb` | Bundled with the Arm GNU Toolchain                | Debugging                       |
| J-Link            | any recent          | [SEGGER](https://www.segger.com/downloads/jlink/) | Flashing and debugging over SWD |

#### **Set 4 - Full Windows host development environment**

For working on every part of the project at once - firmware, [dev tool](#terminology) and `control_api`. Sets 2
and 3 together, spelled out as concrete Windows downloads. One host compiler covers the dev tool, the libraries and
picotool.

| Component         | Where to get it                                                                        | Notes                                                                                                                                   |
|-------------------|----------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------|
| Host GCC 16+      | [winlibs.com](https://winlibs.com/)                                                    | The **UCRT runtime, POSIX threads, SEH** build (`x86_64-ucrt-posix-seh`). A zip - extract it and add `mingw64/bin` to `PATH`, see below |
| Qt 6.10           | [Qt online installer](https://www.qt.io/download-open-source)                          | `mingw_64` kit                                                                                                                          |
| Arm GNU Toolchain | [developer.arm.com](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) | A GCC 16+ build.                                                                                                                        |

Optional components:

| Component         | Where to get it                                            | Needed for                                                  |
|-------------------|------------------------------------------------------------|-------------------------------------------------------------|
| Firmware debugger | Bundled with the Arm GNU Toolchain                         | Debugging                                                   |
| J-Link software   | [segger.com](https://www.segger.com/downloads/jlink/)      | Flashing and debugging over SWD                             |
| CLion             | [jetbrains.com](https://www.jetbrains.com/clion/download/) | IDE builds, see [Building with CLion](#building-with-clion) |

CMake, Python and Git from the table at the top of this section are still needed on top of these. Ninja comes with
WinLibs.

> WinLibs `mingw64/bin` has to sit in a specific place on `PATH`:
> - **Below CMake's `bin`.** WinLibs ships its own `cmake.exe`, which cannot verify HTTPS certificates, so every
    > `FetchContent` download fails with `status_code: 60 ... SSL peer certificate ... was not OK` if it is found first.
> - **Above Qt's `bin`.** Qt ships an older `libstdc++-6.dll`, which the SDK-built picotool crashes on.
>
> Check in a new terminal: `where.exe cmake` must list the CMake install first, and `gcc --version` must report 16.x.

### Building with CMake

All commands run from the `code/` directory.

**Consumer build.** Builds the libraries and `control_api` for your [target](#terminology) - your
[host](#terminology) by default, or another microcontroller acting as a [consumer](#terminology) - plus the
[dev tool](#terminology) when Qt is found:

```bash
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug   # or Release
cmake --build build
```

To build the dev tool, CMake has to find Qt: add `-DCMAKE_PREFIX_PATH=<Qt install>/lib/cmake`
(e.g. `C:/Qt/6.10.0/mingw_64/lib/cmake`). Without it the dev tool is skipped with a warning, unless Qt's `bin` is
on `PATH`.

To pick a specific compiler instead of the first one on `PATH`, add
`-DCMAKE_C_COMPILER=<compiler bin>/gcc -DCMAKE_CXX_COMPILER=<compiler bin>/g++`. Pass both - the C
compiler also builds the assembly, and would otherwise still come from `PATH`.

For another microcontroller, the compiler alone is not enough: CMake also has to know it is cross compiling, or its
compiler check tries to link a normal executable and fails. Pass `-DCMAKE_TOOLCHAIN_FILE=<your toolchain file>`
instead. This path has not been tested yet.

**Firmware build.** The Pico SDK finds the ARM compiler itself, taking the first
`arm-none-eabi-gcc` on `PATH`:

```bash
cmake -G Ninja -B build_fw -DSERVO_CORE_FIRMWARE_BUILD=ON -DCMAKE_BUILD_TYPE=Debug   # or Release
cmake --build build_fw
```

To point it at a specific ARM toolchain, add `-DPICO_TOOLCHAIN_PATH=<Arm GNU Toolchain root>` -
the same way `CMAKE_PREFIX_PATH` picks a Qt.

**Tests:**

```bash
cmake -G Ninja -B build_test -DSERVO_CORE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug   # or Release
cmake --build build_test
ctest --test-dir build_test
```

### Building with CLion

Open the `code/` directory as a CLion project.

#### 1. Register the host toolchain

*Settings -> Build, Execution, Deployment -> Toolchains -> + -> MinGW*

| Field       | Value                                                                      |
|-------------|----------------------------------------------------------------------------|
| Name        | `MinGW-16`                                                                 |
| Environment | your WinLibs directory, e.g. `C:\Tools\Compilers\winlibs-gcc-16.2\mingw64` |

CLion detects the C/C++ compilers and the debugger from that folder. Check that the C++ compiler line
reports **16.x** before continuing.

This toolchain is used for *both* profiles below. The firmware profile needs it too, because the Pico SDK
builds picotool with a [host](#terminology) compiler during the firmware build. The ARM compiler is selected
separately via `PICO_TOOLCHAIN_PATH`.

#### 2. Host profile

*Settings -> Build, Execution, Deployment -> CMake -> +*

| Field           | Value                                                                       |
|-----------------|-----------------------------------------------------------------------------|
| Name            | `Host-GCC16`                                                                |
| Build type      | `Debug` or `Release`                                                        |
| Toolchain       | `MinGW-16`                                                                  |
| Generator       | `Ninja`                                                                     |
| CMake options   | `-DCMAKE_PREFIX_PATH=C:/Qt/6.10.0/mingw_64/lib/cmake` *(adjust to your Qt)* |
| Build directory | `build-host-gcc16`                                                          |

#### 3. Firmware profile

| Field           | Value                                                                                             |
|-----------------|---------------------------------------------------------------------------------------------------|
| Name            | `Firmware-GCC16`                                                                                  |
| Build type      | `Debug` or `Release`                                                                              |
| Toolchain       | `MinGW-16`                                                                                        |
| Generator       | `Ninja`                                                                                           |
| CMake options   | `-DSERVO_CORE_FIRMWARE_BUILD=ON -DPICO_TOOLCHAIN_PATH=<Arm GNU Toolchain root>` *(path optional)* |
| Build directory | `build-fw-gcc16`                                                                                  |

Use a build directory per profile so the two configurations do not fight over one cache.

#### Debugging firmware with J-Link

Add a run configuration for the embedded GDB server (*pull-down next to the build button → Edit Configurations → + →
Embedded GDB Server*):

| Field              | Value                                                                                      |
|--------------------|--------------------------------------------------------------------------------------------|
| Name               | `J-Link`                                                                                   |
| Target remote args | `localhost:2331`                                                                           |
| GDB Server         | path to `JLinkGDBServerCL.exe` (e.g. `C:\Program Files\SEGGER\JLink\JLinkGDBServerCL.exe`) |
| GDB Server args    | `-if SWD -device RP2040_M0_0`                                                              |
| GDB                | path to `arm-none-eabi-gdb.exe` from the Arm GNU Toolchain                                 |

Running this configuration will flash and attach the debugger via J-Link.

SVD files for peripheral register visualization are in `tools/`:

| File               | Use                        |
|--------------------|----------------------------|
| `tools/RP2040.svd` | Current development target |
| `tools/RP2350.svd` | Final production target    |

Point the run configuration's SVD path to the appropriate file to get hardware register views in the debugger.

### CMake options

| Option                                                       | Default | Purpose                                                            |
|--------------------------------------------------------------|---------|--------------------------------------------------------------------|
| `SERVO_CORE_FIRMWARE_BUILD`                                  | OFF     | Switch to firmware/ARM build                                       |
| `SERVO_CORE_BUILD_TESTS`                                     | OFF     | Enable GTest unit tests                                            |
| `ServoCore_ASSERT_LEVEL`                                     | 1       | Assertion level: 0 = disabled, 1 = enabled, 2 = verbose, 3 = debug |
| `SERVO_CORE_CONTROL_API_WINDOWS_COMPORT_DRIVER_DEBUG_PRINTS` | OFF     | Print all bytes passing through the serial driver                  |
| `SERVO_CORE_DISABLE_SERIAL_COMMUNICATION_FRAMEWORK_TIMEOUTS` | OFF     | Disable packet timeouts (debugging aid)                            |

---

## Structure

```
code/
├── common/             # Shared between firmware and consumers
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
│   ├── template/       # Platform-agnostic consumer layer
│   ├── windows/        # Windows implementation
│   └── python/         # Python bindings (planned)
└── dev_tool/           # Qt6 GUI for device control and parameter inspection
```

---

## Architecture

### Serial Communication Framework

`common/libs/serial_communication_framework/`

A binary packet protocol that runs over serial. The framework defines a type-safe command model — each command pairs a
request type and a response type with an operation code, enforced at compile time.

There are two sides: a **master** that sends commands and waits for responses, and a **slave** that
receives commands, dispatches them to registered handlers, and responds. Both sides validate packets using a two-level
CRC (header + payload).

### Protocol

`common/protocol/`

The concrete set of commands the [device](#terminology) supports — ping, parameter read/write, metadata queries, motor
control, etc.
Built on top of the serial communication framework.

### Parameter System

`common/libs/parameter_system/`

A typed parameter system that lets the firmware expose [device](#terminology) state and configuration to
the [consumer](#terminology) over the protocol.
Parameters have a value type (e.g. `uint8`, `float`, `int32`), a name, and a read/write access level. There are three
categories:

- **Saved** — persists across reboots
- **Runtime** — resets on reboot
- **Signal** — read-only, device-generated (sensor data etc.)

The [consumer](#terminology) can enumerate all parameters, query their metadata, and read or write their values at
runtime.

### Control API

`control_api/`

The library used to communicate with a ServoCore [device](#terminology). The design is split into a platform-agnostic
template layer and
platform-specific implementations (currently Windows, Python bindings planned). The template layer is intentionally
portable — it can run on a desktop or be compiled for a microcontroller, so another MCU can act as
the [consumer](#terminology) and
control the ServoCore board.

### Firmware

`firmware/`

The embedded application running on the microcontroller. Handles hardware initialization, drives the motor, reads the
encoder, and processes incoming protocol commands.

Driver implementations are separated from the rest of the firmware via interfaces (`common/drivers/interfaces/`),
keeping hardware-specific code isolated and the rest of the codebase portable.

---

## Coding Conventions

- `K_` prefix for compile-time constants.
- `std::span<uint8_t>` for all buffer passing (zero-copy).
- No heap allocation in firmware — use `StaticList`, `RingBuffer`, and fixed-size arrays.
- Namespaces mirror the directory structure.

### Naming

| Thing                   | Convention                        | Example                     |
|-------------------------|-----------------------------------|-----------------------------|
| Classes & types         | `PascalCase`                      | `SlaveHandler`              |
| Interfaces              | `PascalCase` + `Interface` suffix | `RgbLedInterface`           |
| Functions & methods     | `camelCase`                       | `callMyGoodFunction`        |
| Variables               | `snake_case`                      | `my_variable`               |
| Class members           | `snake_case` + `_` suffix         | `my_member_`                |
| Enum members            | `snake_case`                      | `timed_out`                 |
| Files (single class)    | matches class name                | `SlaveHandler.cpp`          |
| Files (broader/utility) | `snake_case`                      | `serialize_deserialize.cpp` |
