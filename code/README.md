# About

# Building

## Requirements

- ARM gcc compiler / Arm GNU Toolchain
    - The compiler can be downloaded
      from [this link](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
      Choose Bare-metal-target (arm-none-eabi) toolchain for your host platform
    - The bin folder must be added to PATH

## Building directly with cmake

- TODO: ADD THESE INSTRUCTIONS

## Building with Clion IDE

- Open the code directory as Clion project
- Pop up opens to configure Cmake profiles for the project
    - If the popup does not open navigate: File -> Settings -> Build, Execution,
      Deployment -> CMake
- Create profile for both release and debug build:
    - Name: ```Release / Debug```
    - Build type: ```Release/ Debug```
    - Toolchain: ```Use default (will be changed using cmake variable later)```
    - Generator: ```Ninja```
    - CMake options: 
    - Build directory: ```your choice```
    - Leave rest as it is


- Add build configuration
    - TODO: COMPLETE WITH DEBUGGER RUNNING AND SO ON

## List of CMake variables and options
