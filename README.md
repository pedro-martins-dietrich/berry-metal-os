# BerryMetalOS

## Description

**BerryMetalOS** is a bare-metal firmware and operational system, built for a Raspberry Pi 3B.
This project uses assembly and C++ as languages, without any external dependencies.


## Intended Features

- Programming interface for IO components of the board:
  - GPIO;
  - USB;
  - HDMI;
  - 3.5mm audio jack;
- Support to serial communication with UART protocol;
- Basic memory management;
- Programming interface for simple graphical output;
- Simple command line interface;
- Support for running low-level applications in user space.


## Prerequisites

The **GNU cross-compiler toolchain for AArch64** is required to compile and link the kernel image.
The toolchain can be downloaded from the official [Arm Developer website](https://developer.arm.com/tools-and-software/gnu-toolchain).

In order to automate the kernel build, **CMake** is used.
It can be downloaded from the official [CMake website](https://cmake.org/download/).

**Ninja** is recommended as the build system, which can be downloaded from the official [Ninja website](https://ninja-build.org/).

If properly installed, the following commands should report the version for each application:
```bash
cmake --version
ninja --version
aarch64-none-elf-g++ --version
```


## Build

The project can be built by running the following commands:
```bash
cmake -S . -B build/ -G "Ninja" -D CMAKE_TOOLCHAIN_FILE="./toolchain-aarch64.cmake"
cmake --build build/ -j 8
```
_Note: the `-j 8` specifies the number of threads to be used for compiling. Change the value according to your system._


## Loading to the Raspberry Pi 3B

After building, the kernel image (`kernel8.img`) should be present in the `SD Card/` directory.
Copy everything from this directory, paste into the root of the SD card, and insert it into the Raspberry Pi 3B.
