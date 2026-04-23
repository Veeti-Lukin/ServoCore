# ServoCore

ServoCore is a full-stack closed-loop motor controller project, covering custom PCB hardware, embedded firmware, and a Windows host control library with a graphical dev tool.

## System Overview

```
[ Dev Tool (Qt6 GUI) ]
        |
[ Control API (Windows) ]
        |
     Serial (UART)
        |
[ Firmware (RP2040 / RP2350) ]
        |
[ Motor + AS5600L Encoder ]
```

The firmware runs on the microcontroller, manages the motor, and reads position feedback from an AS5600L magnetic encoder. The host communicates with the device over a serial connection using a custom binary packet protocol. The dev tool provides a GUI for device discovery, parameter inspection, and control.

## Hardware

The target MCU is the **RP2350** (with internal flash). Development currently uses the **RP2040**.

The motor position feedback uses an **AS5600L** magnetic encoder.

PCB design (KiCad) lives in `ele/` — early stage, not yet complete.

## Repository Structure

```
ele/        PCB schematic and layout (KiCad)
code/       All software — firmware, host libraries, dev tool
docs/       Datasheets and reference material
```

See [`code/README.md`](code/README.md) for build instructions and software architecture.
