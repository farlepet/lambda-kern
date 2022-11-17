Lambda Kernel
=============

Lambda OS is a hobby operating system developed by Peter Farley. Lambda OS
is designed to be portable to multiple systems, and currently supports
x86-based PCs, ARMv7 (Cortex-A9) support is in progress, and x86_64 and RISCV
(RV64I) support is planned.

**NOTE**: This repository only contains the kernel portion of Lambda OS. For a
usable implementation of the kernel, see the lambda-os repository.

Platform support:
  - x86
     - x86 PC (platform with the most support)
  - ARMv7
     - Cortex-A9
        - Versatile Express A9 (early stages; partially dropped)
     - Cortex-A53
        - Broadcom BCM2836/7 (Raspberry Pi) (early stages)

Building
--------

To be able to build the Lambda kernel, you need the following tools:
  - `gcc`
    - If using `gcc`, you must use a cross-compiler. There is a convenience script for building one: `scripts/build-cross-compiler.sh`
    - TODO: reintroduce clang support
  - `make`
  - `eu-readelf` (elfutils)


To build kernel:
  - (If fisrt time) Clone the `lambda-kern` code repository
  - (If not first time) Run `git pull`
  - Make: `make CROSS_COMPILE=<gcc prefix>`
     - Defaults to x86
     - For clang: `make CC=clang AS=clang`
     - For ARMv7: `make CROSS_COMPILE=<gcc prefix> ARCH=armv7`
        - Using clang with ARMv7 is currently not fully supported
     - To speed up compilation, add `-j<threads>` argument to the `make` command
