# ServoCore

ServoCore is a full-stack closed-loop motor controller project, covering custom PCB hardware, embedded firmware, and control api to integrate ServoCore devices in to your own project.

## System Overview

```
[ Dev Tool (HOST PC GUI) ] [Your own project]
        |
[ Control API]
        |
     Serial (UART)
        |
[ Firmware (RP2040 / RP2350) ]
        |
[ Motor + Encoder ]
```

The firmware runs on the microcontroller, manages the motor, and reads position feedback encoder. The control library communicates with the device over a serial connection using a custom binary packet protocol. The dev tool provides a GUI for device discovery, parameter inspection, and control.

## Hardware

The target MCU is the **RP2350** (with internal flash). Development currently uses the **RP2040**.

The motor position feedback uses an **AS5600L** magnetic encoder.

PCB design (KiCad) lives in `ele/` — early stage, not yet complete.

## Repository Structure

```
ele/        PCB schematic and layout (KiCad)
code/       All software — firmware, control libraries, dev tool
docs/       Datasheets and reference material
```

See [`code/README.md`](code/README.md) for build instructions and software architecture.
